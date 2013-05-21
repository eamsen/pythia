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
#include <string>
#include <vector>
#include <ostream>
#include <fstream>
#include "./request-handler-factory.h"
#include "../io/serialize.h"

using std::string;
using std::vector;
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
DEFINE_string(webcache, "cache/web.bin", "HTML-content cache.");
DEFINE_string(ontologycache, "cache/ontology.bin", "Ontology index cache.");
DEFINE_string(keywordcache, "cache/keywords.bin", "Full text index cache.");

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

  // Load web cache.
  std::ifstream web_cache_stream(FLAGS_webcache);
  if (web_cache_stream) {
    io::Read(web_cache_stream, &web_cache_);
  }
  // Load keyword frequencies.
  std::ifstream keyword_bin_stream(FLAGS_keywordcache);
  if (keyword_bin_stream) {
    // Load keyword frequencies from cached binary format.
    io::Read(keyword_bin_stream, &sum_keyword_freqs_);
    io::Read(keyword_bin_stream, &keyword_freqs_);
    LOG(INFO) << "Keyword index loaded from " << FLAGS_keywordcache << ".";
  } else {
    // Load keyword frequencies from text file.
    sum_keyword_freqs_ = 0;
    std::ifstream keyword_stream("data/word-frequencies.txt");
    string line;
    while (getline(keyword_stream, line)) {
      std::stringstream ss(line);
      string word;
      ss >> word;
      std::transform(word.begin(), word.end(), word.begin(), ::tolower);
      uint32_t freq;
      ss >> freq;
      keyword_freqs_[word] += freq;
      sum_keyword_freqs_ += freq;
    }
    std::ofstream keyword_bin_stream(FLAGS_keywordcache);
    if (keyword_bin_stream) {
      ThreadClock begtime;
      io::Write(sum_keyword_freqs_, keyword_bin_stream);
      io::Write(keyword_freqs_, keyword_bin_stream);
      LOG(INFO) << "Keyword frequencies ("
                << keyword_freqs_.size() << "/" << sum_keyword_freqs_
                << ") saved to " << FLAGS_keywordcache << ".";
    } else {
      LOG(ERROR) << "Could not save ontology index to " << FLAGS_keywordcache
                 << ".";
    }
  }
  // Construct ontology index.
  std::ifstream ontology_bin_stream(FLAGS_ontologycache);
  if (ontology_bin_stream) {
    // Load ontology index from cached binary format.
    ThreadClock begtime;
    ontology_index_.Load(ontology_bin_stream);
    LOG(INFO) << "Ontology index loaded from " << FLAGS_ontologycache
              << " [" << ThreadClock() - begtime << "].";
  } else {
    static const std::unordered_set<string> ontology_filter = {};
        // {"entity", "abstraction", "object", "physicalentity"};
    // Construct ontology from text file.
    ThreadClock begtime;
    std::ifstream ontology_stream("data/ontology-is-a.txt");
    int num_triples = pyt::nlp::OntologyIndex::ParseFromCsv(ontology_stream,
        ontology_filter, &ontology_index_);
    LOG(INFO) << "Indexed " << num_triples << " ontology triples"
              << " [" << ThreadClock() - begtime << "].";
    std::ifstream ontology_scores_stream("data/ontology.entity-scores.txt");
    pyt::nlp::OntologyIndex::ParseScoresFromCsv(ontology_scores_stream,
        &ontology_index_);
    std::ofstream ontology_bin_stream(FLAGS_ontologycache);
    if (ontology_bin_stream) {
      ThreadClock begtime;
      ontology_index_.Save(ontology_bin_stream);
      LOG(INFO) << "Ontology index saved to " << FLAGS_ontologycache
                << " [" << ThreadClock() - begtime << "].";
    } else {
      LOG(ERROR) << "Could not save ontology index to " << FLAGS_ontologycache
                 << ".";
    }
  }
}

Server::~Server() {
  std::ofstream web_cache_stream(FLAGS_webcache);
  if (web_cache_stream) {
    io::Write(web_cache_, web_cache_stream);
  } else {
    LOG(ERROR) << "Could not save web cache to " << FLAGS_webcache << ".";
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
