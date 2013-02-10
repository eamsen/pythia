// Copyright 2012, 2013 Eugen Sawin <esawin@me73.com>
#include "./full-query-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/DefaultHandler.h>
#include <Poco/JSON/Query.h>
#include <glog/logging.h>
#include <flow/clock.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <regex>
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

string StripHtml(const string& content) {
  std::stringstream ss;
  size_t pos = content.find("<body");
  while (pos != string::npos && content.substr(pos, 6) != "</body") {
    const size_t beg = content.find(">", pos) + 1;
    pos = content.find("<", beg);
    if (beg != string::npos) {
      ss << content.substr(beg, pos - beg);
    }
  }
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
  auto items = json_query.findArray("items");
  NamedEntityExtractor extractor;
  vector<vector<std::pair<string, Entity::Type> > > extracted(items->size());
  // #pragma omp parallel for
  for (size_t i = 0; i < items->size(); ++i) {
    const string& url = items->getObject(i)->getValue<string>("link");
    // LOG(INFO) << "Processing " << url;
    ThreadClock clock2;
    string content = StripHtml(HttpGetRequest(url, timeout));
    // #pragma omp critical
    http_get_time += ThreadClock() - clock2;
    clock2 = ThreadClock();
    // extractor.Extract(content, &index);
    extractor.Extract(content, &extracted[i]);
    // #pragma omp critical
    ner_time += ThreadClock() - clock2;
  }
  for (const auto& vec: extracted) {
    for (const auto& e: vec) {
      index.Add(e.first, e.second);
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

  const OntologyIndex& ontology = server_.OntologyIndex();
  // Find the top candidates.
  size_t num_top = 30;
  std::unordered_map<string, int> entities;
  string top_candidates;
  while (num_top && index.QueueSize()) {
    Entity entity = index.PopTop();
    if (entities.count(entity.name) || IsSimilarToQuery(entity.name) ||
        IsBadName(entity.name) ||
        ontology.LhsNameId(entity.name) == OntologyIndex::kInvalidId) {
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
    entities.insert({entity.name, index.Frequency(entity)});
  }

  // Find target types.
  vector<string> target_types;
  for (const string& w: target_keywords) {
    // Add types occuring on the right-hand side of an is-a relation, which are
    // singular forms of the target keywords.
    const vector<string> singulars = SingularForms(w);
    for (const string& s: singulars) {
      // Check if a singular form of a target keyword is an ontology class.
      if (ontology.RhsNameId(s) != OntologyIndex::kInvalidId) {
        target_types.push_back(s);
      }
    }
  }
  // TODO(esawin): Refactor this out of here.
  // Here comes the target type guessing based on most frequent entity types.
  const int is_a_relation_id = ontology.RelationId("is-a");
  DLOG_IF(FATAL, is_a_relation_id == OntologyIndex::kInvalidId)
      << "is-a relation is not not indexed.";
  vector<std::pair<int, float> > rhs_freqs;
  for (const auto& e: entities) {
    const auto relations = ontology.RelationsByLhs(e.first);
    for (const auto& r: relations) {
      if (r.first == is_a_relation_id) {
        rhs_freqs.push_back({r.second, e.second});
      }
    }
  }
  std::sort(rhs_freqs.begin(), rhs_freqs.end());
  vector<std::pair<float, int> > rhs_sorted;
  const float log_num_triples = std::log2(ontology.NumTriples());
  LOG(INFO) << "Number of triples: " << ontology.NumTriples()
            << " (" << log_num_triples << ").";
  for (const auto& rhs: rhs_freqs) {
    const float idf = log_num_triples - std::log2(ontology.RhsFreq(rhs.first));
    const float score = idf * rhs.second;
    if (rhs_sorted.size() && rhs_sorted.back().second == rhs.first) {
        rhs_sorted.back().first += score;
    } else {
      rhs_sorted.push_back({score, rhs.first});
    }
  }
  const size_t k = std::min(4u, rhs_sorted.size());
  std::partial_sort(rhs_sorted.begin(), rhs_sorted.begin() + k,
                    rhs_sorted.end(), std::greater<std::pair<float, int> >());
  rhs_sorted.resize(k);
  for (const auto& rhs: rhs_sorted) {
    target_types.push_back(ontology.Name(rhs.second));
    LOG(INFO) << rhs.second << ": " << rhs.first;
  }
  LOG(INFO) << "Top candidates: " << top_candidates;
  response_stream << "],\"target_types\":"
                  << JsonArray(target_types.begin(), target_types.end());
  response_stream << "}";
}

}  // namespace net
}  // namespace pyt
