// Copyright 2012, 2013 Eugen Sawin <esawin@me73.com>
#include "./full-query-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/DefaultHandler.h>
#include <Poco/JSON/Query.h>
#include <glog/logging.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <flow/clock.h>
#include "./server.h"
#include "./query-parser.h"
#include "./http-request.h"
#include "../nlp/entity-index.h"
#include "../nlp/named-entity-extractor.h"
#include "../nlp/edit-distance.h"
#include "../nlp/grammar-tools.h"

using std::string;
using std::vector;
using pyt::nlp::Tagger;
using pyt::nlp::NamedEntityExtractor;
using pyt::nlp::EntityIndex;
using pyt::nlp::Entity;
using pyt::nlp::EditDistance;
using pyt::nlp::PrefixEditDistance;
using pyt::nlp::OntologyIndex;
using pyt::nlp::SingularForms;
using flow::ThreadClock;
using flow::ClockDiff;

namespace pyt {
namespace net {

template<typename It>
string JsonArray(It begin, It end) {
  std::stringstream ss;
  ss << "[";
  It it = begin;
  while (it != end) {
    if (it != begin) {
      ss << ",";
    }
    ss << "\"" << *it << "\"";
    ++it;
  }
  ss << "]";
  return ss.str();
}

FullQueryRequestHandler::FullQueryRequestHandler(const Poco::URI& uri)
    : server_(static_cast<Server&>(Poco::Util::Application::instance())),
      uri_(uri),
      query_analyser_(server_.Tagger()) {}

void FullQueryRequestHandler::Handle(Request* request, Response* response) {
  static const int64_t timeout = 3 * 1e6;  // Microseconds.
  const Query query(uri_.getQuery());
  const string& query_text = query.Text("qf");
  const string& query_uri = query.Uri("qf");
  LOG(INFO) << "Full query request: " << query_text << ".";

  // Get the target keywords.
  vector<string> target_keywords = query_analyser_.TargetKeywords(query_text);
  LOG(INFO) << "Target keywords: " << JsonArray(target_keywords.begin(),
                                                target_keywords.end());

  ThreadClock clock;
  // Get Google search results.
  string response_data = HttpsGetRequest(server_.SearchHost() +
                                         server_.SearchBase() + query_uri,
                                         timeout);
  DLOG(INFO) << "Google search time [" << ThreadClock() - clock << "].";
  clock = ThreadClock();
  Poco::JSON::Parser json_parser;
  Poco::JSON::DefaultHandler json_handler;
  json_parser.setHandler(&json_handler);
  json_parser.parse(response_data);
  Poco::Dynamic::Var json_data = json_handler.result();
  Poco::JSON::Query json_query(json_data);
  DLOG(INFO) << "POCO JSON parse time [" << ThreadClock() - clock << "].";
  clock = ThreadClock();

  ClockDiff http_get_time = 0;
  ClockDiff ner_time = 0;
  // Get page contents and extract named entities.
  EntityIndex index;
  NamedEntityExtractor extractor;
  auto items = json_query.findArray("items");
  for (size_t end = items->size(), i = 0; i < end; ++i) {
    const string& url = items->getObject(i)->getValue<string>("link");
    // LOG(INFO) << "Processing " << url;
    clock = ThreadClock();
    string content = HttpGetRequest(url, timeout);
    http_get_time += ThreadClock() - clock;
    auto start = content.find("<body");
    if (start != string::npos) {
      auto end = content.find("</body", start + 5);
      content = content.substr(start, end - start);
      clock = ThreadClock();
      extractor.Extract(content, &index);
      ner_time += ThreadClock() - clock;
    }
  }
  DLOG(INFO) << "Total HTTP-Get time [" << http_get_time << "].";
  DLOG(INFO) << "Total NER time [" << ner_time << "].";

  // Assemble the response.
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  std::ostream& response_stream = response->send();
  response_stream << "{\"results\":";
  // items->stringify(response_stream, 0);
  response_stream << "[]";
  response_stream << ","
      << "\"target_keywords\":"
      << JsonArray(target_keywords.begin(), target_keywords.end()) << ","
      << "\"entities\":[";
  const vector<string>& query_words = query.Words("qf");

  auto IsSimilarToQuery = [&query_words](const string& word) {
    for (const string& w: query_words) {
      const int ped = PrefixEditDistance(w, word);
      if (ped <= std::abs((w.size() - 1) / 3)) {
        return true;
      }
    }
    return false;
  };

  auto IsBadName = [](const string& word) {
    for (const char c: word) {
      if (!std::isalpha(c) && c != ' ' && c != '-' && c != '\'') {
        return true;
      }
    }
    return word.size() < 2 || word.size() > 40;
  };

  // Find the top candidates.
  size_t num_top = 30;
  std::unordered_set<string> entities;
  string top_candidates;
  while (num_top && index.QueueSize()) {
    Entity entity = index.PopTop();
    if (entities.count(entity.name) || IsSimilarToQuery(entity.name) ||
        IsBadName(entity.name)) {
      // DLOG(INFO) << "Filtered entity: " << entity.name;
      continue;
    }
    top_candidates += (top_candidates.size() ? ", ": "") + entity.name;
    --num_top;
    if (entities.size()) {
      response_stream << ",";
    }
    response_stream << "{\"name\":\"" << entity.name
                    << "\",\"type\":\"" << Entity::TypeName(entity.type)
                    << "\",\"score\":" << index.Frequency(entity) << "}";
    entities.insert(entity.name);
  }

  // Find target type.
  vector<string> target_types;
  for (const string& w: target_keywords) {
    const vector<string> singulars = SingularForms(w);
    for (const string& s: singulars) {
      // Check if a singular form of a target keyword is an ontology class.
      if (server_.OntologyIndex().RhsNameId(s) != OntologyIndex::kInvalidId) {
        target_types.push_back(s);
      }
    }
  }
  LOG(INFO) << "Top candidates: " << top_candidates;
  response_stream << "],\"target_types\":"
                  << (target_types.size() ?
                      JsonArray(target_types.begin(), target_types.end()) :
                      "[\"unknown\"]");
  response_stream << "}";
}

}  // namespace net
}  // namespace pyt
