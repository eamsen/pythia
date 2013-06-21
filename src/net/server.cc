// Copyright 2012, 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_SERVER_APPLICATION_H_
#define SRC_NET_SERVER_APPLICATION_H_

#include "./server.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/ThreadPool.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <flow/clock.h>
#include <flow/string.h>
#include <string>
#include <vector>
#include <ostream>
#include <fstream>
#include <sstream>
#include "./request-handler-factory.h"
#include "../io/serialize.h"

using std::string;
using std::vector;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using Poco::Net::ServerSocket;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using flow::time::ThreadClock;
using pyt::nlp::Tagger;

namespace pyt {
namespace net {

DEFINE_string(api, "api.txt", "Google API key + CX file.");
DEFINE_string(groundtruthdir, "ground-truth", "Ground truth directory.");
DEFINE_string(webcache, "cache/web.bin", "HTML-content cache.");
DEFINE_string(ontologycache, "cache/ontology.bin", "Ontology index cache.");
DEFINE_string(keywordcache, "cache/keywords.bin", "Full text index cache.");
DEFINE_string(entitycache, "cache/entity.bin", "Extracted entities cache.");
DEFINE_string(groundtruthcache, "cache/ground-truth.bin",
    "Ground truth index cache.");

Server::Server(const string& name, const string& version,
               const string& doc_path, const uint16_t port,
               const uint16_t threads, const uint16_t queue_size)
    : name_(name),
      version_(version),
      doc_path_(doc_path),
      port_(port),
      num_threads_(threads),
      queue_size_(queue_size),
      tagger_(Tagger::kPos) {
  // Read Google API file.
  std::fstream file(FLAGS_api);
  LOG_IF(FATAL, !file.good()) << "Google API file " << FLAGS_api
                               << " not found.";
  std::getline(file, api_key_);
  LOG_IF(FATAL, file.eof()) << "Provided Google API file " << FLAGS_api
                            << " has invalid format. Should be <key>\n<cx>.";
  std::getline(file, api_cx_);
  search_host_ = "https://www.googleapis.com";
  search_base_ = "/customsearch/v1?";
  search_base_ += "key=" + api_key_;
  search_base_ += "&cx=" + api_cx_;
  search_base_ += "&q=";

  fb_base_ = "/freebase/v1/search?limit=20&lang=en&key=";
  fb_base_ += api_key_ + "&query=";

  // Load ground truth.
  {
    ThreadClock start_time;
    ifstream bin_stream(FLAGS_groundtruthcache);
    if (bin_stream) {
      // Load binary ground truth index.
      io::Read(bin_stream, &ground_truth_);
      LOG(INFO) << "Read binary ground truth index ["
          << ThreadClock() - start_time << "]";
    } else {
      // Construct ground truth index from text files.
      for (int i = 0; i < 10; ++i) {
        ifstream stream(FLAGS_groundtruthdir + "/entities-" + std::to_string(i));
        if (!stream) {
          continue;
        }
        stringstream json;
        json << "[";
        string prev_query;
        while (stream.good()) {
          string query;
          stream >> query;

          string entity;
          stream >> entity;

          flow::string::Replace("-", " ", &query);
          flow::string::Replace("_", " ", &entity);

          if (query.empty() || entity.empty()) {
            continue;
          }

          if (query != prev_query) {
            if (prev_query.size()) {
              json << "],";
            }
            json << "[\"" << query << "\",";
          } else if (prev_query.size()) {
            json << ",";
          }
          json << "\"" << entity << "\"";
          prev_query.swap(query);
        }
        if (prev_query.size()) {
          json << "]";
        }
        json << "]";
        LOG(INFO) << json.str();
        ground_truth_.insert({std::to_string(i), json.str()});
      }
      ofstream out_bin_stream(FLAGS_groundtruthcache);
      io::Write(ground_truth_, out_bin_stream);
      LOG(INFO) << "Constructed ground truth index ["
          << ThreadClock() - start_time << "]";
    }
  }

  // Load web cache.
  {
    ifstream stream(FLAGS_webcache);
    if (stream) {
      io::Read(stream, &web_cache_);
    }
  }

  // Load entity cache.
  {
    ifstream stream(FLAGS_entitycache);
    if (stream) {
      // TODO(esawin): Fix serialization (flow) first.
      // io::Read(entity_cache_stream, &entity_cache_);
    }
  }

  // Load keyword frequencies.
  {
    ifstream bin_stream(FLAGS_keywordcache);
    if (bin_stream) {
      // Load keyword frequencies from cached binary format.
      io::Read(bin_stream, &sum_keyword_freqs_);
      io::Read(bin_stream, &keyword_freqs_);
      LOG(INFO) << "Keyword index loaded from " << FLAGS_keywordcache << ".";
    } else {
      // Load keyword frequencies from text file.
      sum_keyword_freqs_ = 0;
      ifstream stream("data/word-frequencies.txt");
      string line;
      while (getline(stream, line)) {
        stringstream ss(line);
        string word;
        ss >> word;
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        uint32_t freq;
        ss >> freq;
        keyword_freqs_[word] += freq;
        sum_keyword_freqs_ += freq;
      }
      std::ofstream out_bin_stream(FLAGS_keywordcache);
      if (out_bin_stream) {
        ThreadClock start_time;
        io::Write(sum_keyword_freqs_, out_bin_stream);
        io::Write(keyword_freqs_, out_bin_stream);
        LOG(INFO) << "Keyword frequencies ("
                  << keyword_freqs_.size() << "/" << sum_keyword_freqs_
                  << ") saved to " << FLAGS_keywordcache << ".";
      } else {
        LOG(ERROR) << "Could not save keyword index to " << FLAGS_keywordcache
                   << ".";
      }
    }
  }

  // Construct ontology index.
  {
    ifstream bin_stream(FLAGS_ontologycache);
    if (bin_stream) {
      // Load ontology index from cached binary format.
      ThreadClock start_time;
      ontology_index_.Load(bin_stream);
      LOG(INFO) << "Ontology index loaded from " << FLAGS_ontologycache
                << " [" << ThreadClock() - start_time << "]";
    } else {
      static const std::unordered_set<string> ontology_filter = {};
          // {"entity", "abstraction", "object", "physicalentity"};
      // Construct ontology from text file.
      ThreadClock start_time;
      ifstream stream("data/ontology-is-a.txt");
      int num_triples = pyt::nlp::OntologyIndex::ParseFromCsv(stream,
          ontology_filter, &ontology_index_);
      LOG(INFO) << "Indexed " << num_triples << " ontology triples"
                << " [" << ThreadClock() - start_time << "]";
      ifstream scores_stream("data/ontology.entity-scores.txt");
      pyt::nlp::OntologyIndex::ParseScoresFromCsv(scores_stream,
          &ontology_index_);
      ofstream out_bin_stream(FLAGS_ontologycache);
      if (out_bin_stream) {
        ThreadClock start_time;
        ontology_index_.Save(out_bin_stream);
        LOG(INFO) << "Ontology index saved to " << FLAGS_ontologycache
                  << " [" << ThreadClock() - start_time << "]";
      } else {
        LOG(ERROR) << "Could not save ontology index to " << FLAGS_ontologycache
                   << ".";
      }
    }
  }
}

Server::~Server() {
  ofstream web_cache_stream(FLAGS_webcache);
  if (web_cache_stream) {
    io::Write(web_cache_, web_cache_stream);
  } else {
    LOG(ERROR) << "Could not save web cache to " << FLAGS_webcache << ".";
  }
  ofstream entity_cache_stream(FLAGS_entitycache);
  if (entity_cache_stream) {
    // TODO(esawin): Fix serialization (flow) first.
    // io::Write(entity_cache_, entity_cache_stream);
  } else {
    LOG(ERROR) << "Could not save entity cache to " << FLAGS_entitycache << ".";
  }
}

void Server::Run() {
  vector<string> args = {"pythia"};
  run(args);
}

const string& Server::DocumentPath() const {
  return doc_path_;
}

const string& Server::ApiKey() const {
  return api_key_;
}

const string& Server::ApiCx() const {
  return api_cx_;
}

const string& Server::SearchHost() const {
  return search_host_;
}

const string& Server::SearchBase() const {
  return search_base_;
}

const string& Server::FreebaseBase() const {
  return fb_base_;
}

const string& Server::GroundTruth(const string& path) const {
  static const string kEmpty = "[]";
  const auto it = ground_truth_.find(path);
  if (it == ground_truth_.end()) {
    return kEmpty;
  }
  return it->second;
}

const Tagger& Server::Tagger() const {
  return tagger_;
}

const nlp::OntologyIndex& Server::OntologyIndex() const {
  return ontology_index_;
}

const uint32_t Server::SumKeywordFreqs() const {
  return sum_keyword_freqs_;
}

uint32_t Server::KeywordFreq(const std::string& name) const {
  const auto it = keyword_freqs_.find(name);
  if (it == keyword_freqs_.end()) {
    return 0;
  }
  return it->second;
}

std::unordered_map<string, string>& Server::WebCache() {
  return web_cache_;
}

std::unordered_map<std::string,
    std::vector<std::pair<std::string,
                pyt::nlp::Entity::Type>>>& Server::EntityCache() {
  return entity_cache_;
}

void Server::initialize(Application& self) {  // NOLINT
  ServerApplication::initialize(self);
  Poco::Net::SSLManager::instance().initializeClient(
      0, 0, new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "",
                                   Poco::Net::Context::VERIFY_NONE, 9, false,
                                   "ALL:!ADH:!LOW:!EXP!MD5:@STRENGTH"));
}

void Server::uninitialize() {
  ServerApplication::uninitialize();
}

int Server::main(const vector<string>& args) {
  ServerSocket socket(port_);
  HTTPServerParams* params = new HTTPServerParams();
  params->setServerName(name_);
  params->setSoftwareVersion(version_);
  params->setMaxQueued(queue_size_);
  params->setMaxThreads(num_threads_);
  HTTPServer server(new RequestHandlerFactory(), socket, params);
  server.start();
  waitForTerminationRequest();
  server.stop();
  return Application::EXIT_OK;
}

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_SERVER_APPLICATION_H_
