// Copyright 2012, 2013 Eugen Sawin <esawin@me73.com>
#include "./type-info-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/DefaultHandler.h>
#include <Poco/JSON/Query.h>
#include <glog/logging.h>
#include <flow/clock.h>
#include <flow/string.h>
#include <flow/stringify.h>
// #include <flow/io/html.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <tuple>
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
using std::array;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::tuple;
using std::make_tuple;
using std::get;
using std::log2;
using std::max;
using std::min;
using pyt::nlp::Tagger;
using pyt::nlp::NamedEntityExtractor;
using pyt::nlp::EntityIndex;
using pyt::nlp::Entity;
using pyt::nlp::EditDistance;
using pyt::nlp::PrefixEditDistance;
using pyt::nlp::OntologyIndex;
using pyt::nlp::SingularForms;
using flow::time::ThreadClock;
using flow::time::Clock;
using flow::time::ClockDiff;
using flow::io::JsonArray;

namespace pyt {
namespace net {

TypeInfoRequestHandler::TypeInfoRequestHandler(const Poco::URI& uri)
    : server_(static_cast<Server&>(Poco::Util::Application::instance())),
      uri_(uri) {}

void TypeInfoRequestHandler::Handle(Request* request, Response* response) {
  static const int64_t timeout = 3 * 1e6;  // Microseconds.

  // Prepare clocks.
  Clock request_start_time;
  Clock start_time;
  Clock end_time;

  // Prepare response stream.
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  std::ostream& response_stream = response->send();

  const Query query(uri_.getQuery());
  const string& query_text = query.Text("ti");
  vector<string> entities = flow::string::Split(query_text, "+");
  LOG(INFO) << "Type Info request: " << query_text;

  auto& web_cache = server_.WebCache();
  const OntologyIndex& ontology = server_.OntologyIndex();

  response_stream << "{";

  // Find target types.
  // (type name, score, global frequency).
  vector<tuple<string, int, int>> target_types;

  // Here comes the target type guessing based on most frequent entity types.
  const int is_a_relation_id = ontology.RelationId("is-a");
  DLOG_IF(FATAL, is_a_relation_id == OntologyIndex::kInvalidId)
      << "is-a relation is not indexed.";
  vector<tuple<string, unordered_map<string, int>>> entity_types;
  for (string& e: entities) {
    flow::string::Replace("\"", "", &e);
    string space_less = e;
    flow::string::Replace(" ", "", &space_less);
    entity_types.push_back(make_tuple(e, unordered_map<string, int>()));
    const auto& relations = ontology.RelationsByLhs(space_less);
    for (const auto& r: relations) {
      if (r.first == is_a_relation_id) {
        get<1>(entity_types.back())[ontology.Name(r.second)] =
            ontology.RhsFrequency(r.second);
      }
    }
  }
  response_stream << "\"yago_types\":" << JsonArray(entity_types);
  // std::sort(rhs_freqs.begin(), rhs_freqs.end());
  // vector<std::pair<float, tuple<int, int>> > rhs_sorted;
  // Higher values increase score for more abstract types.
  // const float log_num_triples = 23.0f;
  // for (const auto& rhs: rhs_freqs) {
    // const float idf = log_num_triples -
        // std::log2(ontology.RhsFrequency(rhs.first));
    // const float score = idf * rhs.second;
    // if (rhs_sorted.size() && get<0>(rhs_sorted.back().second) == rhs.first) {
        // rhs_sorted.back().first += score * 2.0f;
    // } else {
      // rhs_sorted.push_back({score,
          // make_tuple(rhs.first, ontology.RhsFrequency(rhs.first))});
    // }
  // }
  // const size_t k = std::min(3ul, rhs_sorted.size());
  // if (k > 0) {
    // std::partial_sort(rhs_sorted.begin(), rhs_sorted.begin() + k,
                      // rhs_sorted.end(),
                      // std::greater<std::pair<float, tuple<int, int>> >());
    // rhs_sorted.resize(k);
    // for (const auto& rhs: rhs_sorted) {
      // target_types.push_back(make_tuple(ontology.Name(get<0>(rhs.second)),
                                        // rhs.first, get<1>(rhs.second)));
    // }
  // }

  // // The target types based on on Freebase.
  // vector<tuple<string, int, int>> fb_target_types;
  // std::unordered_map<string, float> entity_type_scores; 
  // if (query.Words("fbtt").size() && query.Words("fbtt")[0] == "1") {
  //   for (const auto& e: entities) {
  //     string name = e.first;
  //     flow::string::Replace(" ", "+", &name);
  //     const string fb_url = server_.SearchHost() + server_.FreebaseBase() +
  //         "\"" + name + "\"";
  //     auto result_it = web_cache.find(fb_url);
  //     if (result_it == web_cache.end()) {
  //       result_it = web_cache.insert({fb_url,
  //                                     HttpsGetRequest(fb_url, timeout)}).first;
  //     }
  //     if (result_it->second.size() == 0 || result_it->second[0] != '{') {
  //       LOG(WARNING) << "Freebase return error for " << name << ".";
  //       continue;
  //     }
  //     // Don't trust the search results and JSON parsing.
  //     try {
  //       Poco::Dynamic::Var json_data;
  //       json_parser.parse(result_it->second);
  //       json_data = json_handler.result();
  //       const auto object = json_data.extract<Poco::JSON::Object::Ptr>();
  //       if (!object) {
  //         continue;
  //       }
  //       const string status = object->get("status").convert<string>(); 
  //       if (status != "200 OK") {
  //         continue;
  //       }
  //       const auto items =
  //           object->get("result").extract<Poco::JSON::Array::Ptr>();
  //       for (size_t i = 0; i < items->size(); ++i) {
  //         const auto notable = items->getObject(i)->getObject("notable");
  //         const float type_score =
  //             items->getObject(i)->get("score").convert<float>();
  //         if (!notable) {
  //           continue;
  //         }
  //         const string type = notable->getValue<string>("name");
  //         entity_type_scores[type] += type_score;
  //       }
  //     } catch(const Poco::Exception& e) {
  //       LOG(WARNING) << e.what();
  //     } catch(const std::exception& e) {
  //       LOG(WARNING) << e.what();
  //     } catch (...) {
  //       LOG(WARNING) << "Unknown exception occured.";
  //     }
  //   }
  //   const size_t k = std::min(3ul, entity_type_scores.size());
  //   if (k > 0) {
  //     vector<std::pair<float, string>> sorted_entity_type_scores;
  //     sorted_entity_type_scores.reserve(entity_type_scores.size());
  //     for (const auto& p: entity_type_scores) {
  //       sorted_entity_type_scores.push_back({p.second, p.first});
  //     }
  //     std::partial_sort(sorted_entity_type_scores.begin(),
  //                       sorted_entity_type_scores.begin() + k,
  //                       sorted_entity_type_scores.end(),
  //                       std::greater<std::pair<float, string>>());
  //     sorted_entity_type_scores.resize(k);
  //     for (const auto& e: sorted_entity_type_scores) {
  //       fb_target_types.push_back(make_tuple(e.second, e.first, 0));
  //     }
  //   }
  // }
  // end_time = Clock();

  // LOG(INFO) << "Target types: " << JsonArray(target_types);
  // LOG(INFO) << "Freebase target types: " << JsonArray(fb_target_types);
  // response_stream << ",\"semantic_query\":{"
  //     << "\"duration\":" << (end_time - start_time).Value()
  //     << ",\"target_types\":" << JsonArray(target_types)
  //     << ",\"fb_target_types\":" << JsonArray(fb_target_types);

  // // Construct the Broccoli query.
  // response_stream << ",\"broccoli_query\":\"$1 :r:is-a "
  //     << (target_types.size() ? get<0>(target_types[0]) : "unknown")
  //     << "; $1 :r:occurs-with "
  //     << flow::io::Str(keywords, " ", "", "", "", "", "", "")
  //     << "\"}";

  // end_time = Clock();
  // response_stream << ",\"duration\":"
  //    << (end_time - request_start_time).Value() << "}";
  response_stream << "}";
}

}  // namespace net
}  // namespace pyt
