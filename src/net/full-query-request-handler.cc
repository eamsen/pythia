// Copyright 2012, 2013 Eugen Sawin <esawin@me73.com>
#include "./full-query-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/DefaultHandler.h>
#include <Poco/JSON/Query.h>
#include <glog/logging.h>
#include <flow/clock.h>
#include <flow/string.h>
#include <flow/stringify.h>
#include <flow/io/html.h>
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

string StripHtml(const string& content) {
  std::stringstream ss;
  size_t pos = content.find("<body");
  const size_t end = content.find("</body", pos);
  while (pos != string::npos && pos < end) {
    const size_t beg = content.find(">", pos);
    pos = content.find("<", beg);
    if (beg != string::npos) {
      ss << content.substr(beg + 1, pos - beg - 1);
    }
  }
  return ss.str();
}

struct EntityItem {
  static string JsonArray(const vector<EntityItem>& items) {
    std::ostringstream ss;
    ss << "[";
    for (auto it = items.begin(), end = items.end(); it != end; ++it) {
      if (it != items.begin()) {
        ss << ",";
      }
      ss << it->JsonArray();
    }
    ss << "]";
    return ss.str();
  }

  string JsonArray() const {
    std::ostringstream ss;
    ss << "[\"" << name << "\",\"" << Entity::TypeName(coarse_type) << "\","
       << content_freq << "," << snippet_freq << "," << corpus_freq << ","
       << score << ","
       << flow::io::JsonArray(content_index) << ","
       << flow::io::JsonArray(snippet_index) << "]";
    return ss.str();
  }

  string name;
  Entity::Type coarse_type;
  int content_freq;
  int snippet_freq;
  int corpus_freq;
  float score;
  vector<pair<int, int>> content_index;
  vector<pair<int, int>> snippet_index;
};

int FilterEntityItems(vector<EntityItem>* items) {
  static const size_t kMinDocFreq = 2;

  const size_t org_size = items->size();
  size_t i = 0;
  while (i < items->size()) {
    if (std::max((*items)[i].content_index.size(),
                 (*items)[i].snippet_index.size()) < kMinDocFreq) {
      (*items)[i] = items->back();
      items->pop_back();
    } else {
      ++i;
    }
  }
  return org_size - items->size();
}

FullQueryRequestHandler::FullQueryRequestHandler(const Poco::URI& uri)
    : server_(static_cast<Server&>(Poco::Util::Application::instance())),
      uri_(uri),
      query_analyser_(server_.Tagger()) {}

// TODO(esawin): This needs refactoring.
void FullQueryRequestHandler::Handle(Request* request, Response* response) {
  static const int64_t timeout = 3 * 1e6;  // Microseconds.

  Clock request_start_time;
  Clock start_time;
  Clock end_time;
  // Prepare response stream.
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  std::ostream& response_stream = response->send();

  const Query query(uri_.getQuery());
  const string& query_text = query.Text("qf");
  const string& query_uri = query.Uri("qf");
  LOG(INFO) << "Full query request: " << query_text << ".";

  // Get the target keywords.
  vector<string> target_keywords = query_analyser_.TargetKeywords(query_text);
  vector<string> keywords = query_analyser_.Keywords(query_text,
      target_keywords);
  LOG(INFO) << "Keywords: " << keywords;
  LOG(INFO) << "Target keywords: " << target_keywords;

  end_time = Clock();

  response_stream << "{";
  if (query.Text("eval") != "") {
    response_stream << "\"eval\":" << query.Text("eval") << ",";
  }
  response_stream << "\"query_analysis\":{"
      << "\"duration\":" << (end_time - start_time).Value() << ","
      << "\"query\":" << JsonArray(query.Words("qf")) << ","
      << "\"keywords\":" << JsonArray(keywords) << ","
      << "\"target_keywords\":" << JsonArray(target_keywords) << "}";

  start_time = end_time;

  auto& web_cache = server_.WebCache();
  // Get Google search results.
  const string search_url = server_.SearchHost() + server_.SearchBase() +
      query_uri;
  auto response_it = web_cache.find(search_url);
  if (response_it == web_cache.end()) {
    response_it = web_cache.insert({search_url,
        HttpsGetRequest(search_url, timeout)}).first;
  }
  try {
  Poco::JSON::Parser json_parser;
  Poco::JSON::DefaultHandler json_handler;
  json_parser.setHandler(&json_handler);
  json_parser.parse(response_it->second);
  Poco::Dynamic::Var json_data = json_handler.result();
  const auto object = json_data.extract<Poco::JSON::Object::Ptr>();
  const auto items = object->get("items").extract<Poco::JSON::Array::Ptr>();

  EntityIndex index;
  const int num_items = items->size();
  // Get document contents.
  {
    vector<tuple<string, string>> documents(num_items);
    #pragma omp parallel for
    for (int i = 0; i < num_items; ++i) {
      const string& url = items->getObject(i)->getValue<string>("link");
      if (web_cache.find(url) == web_cache.end()) {
        documents[i] = make_tuple(url,
            flow::io::StripHtml(HttpGetRequest(url, timeout)));
      }
    }
    for (int i = 0; i < num_items; ++i) {
      const string& url = items->getObject(i)->getValue<string>("link");
      if (web_cache.find(url) == web_cache.end()) {
        web_cache.insert({get<0>(documents[i]), get<1>(documents[i])});
        const string& snippet = items->getObject(i)->getValue<string>("snippet");
        web_cache.insert({"snippet/" + url, snippet});
      }
    }
  } 
  end_time = Clock();

  response_stream << ",\"document_retrieval\":{"
      << "\"duration\":" << (end_time - start_time).Value()
      << ",\"documents\":";
  items->stringify(response_stream, 0);
  response_stream << "}";

  start_time = end_time;

  auto IsBadName = [](const string& word) {
    for (const char c: word) {
      if (!std::isalpha(c) && c != ' ' && c != '-' && c != '\'') {
        return true;
      }
    }
    return word.size() < 2 || word.size() > 30;
  };

  auto& entity_cache = server_.EntityCache();
  const OntologyIndex& ontology = server_.OntologyIndex();

  // Extract named entities.
  unordered_map<string, int> entity_ids;
  vector<EntityItem> entity_items;
  vector<array<int, 4>> coarse_type_freqs;
  vector<vector<pair<string, Entity::Type>>> extracted_content(num_items);
  vector<vector<pair<string, Entity::Type>>> extracted_snippets(num_items);
  {
    #pragma omp parallel for
    for (int i = 0; i < num_items; ++i) {
      NamedEntityExtractor extractor;
      const string& url = items->getObject(i)->getValue<string>("link");
      const auto entity_it = entity_cache.find(url);
      if (entity_it == entity_cache.end()) {
        const auto content_it = web_cache.find(url);
        const auto snippet_it = web_cache.find("snippet/" + url);
        extractor.Extract(content_it->second, &extracted_content[i]);
        extractor.Extract(snippet_it->second, &extracted_snippets[i]);
        #pragma omp critical
        {
          entity_cache.insert({url, extracted_content[i]});
          entity_cache.insert({"snippet/" + url, extracted_snippets[i]});
        }
      } else {
        const auto snippet_it = entity_cache.find("snippet/" + url);
        extracted_content[i] = entity_it->second;
        extracted_snippets[i] = snippet_it->second;
      }
    }
    for (int i = 0; i < num_items; ++i) {
      for (auto p: extracted_content[i]) {
        flow::string::Replace("\"", "", &p.first);
        if (IsBadName(p.first)) {
          continue;
        }
        std::transform(p.first.begin(), p.first.end(), p.first.begin(),
            ::tolower);
        string space_less = p.first;
        flow::string::Replace(" ", "", &space_less);
        const int ontology_id = ontology.LhsNameId(space_less);
        auto it = entity_ids.find(p.first);
        if (it == entity_ids.end()) {
          it = entity_ids.insert({p.first, entity_items.size()}).first;
          entity_items.push_back({p.first, p.second, 0, 0,
                                  ontology.LhsFrequency(ontology_id),
                                  0.0f, {}, {}});
          coarse_type_freqs.push_back({{0, 0, 0, 0}});
        } 
        EntityItem& item = entity_items[it->second];
        ++item.content_freq;
        if (item.content_index.empty() ||
            item.content_index.back().first != i) {
          item.content_index.push_back({i, 0});
        } 
        ++item.content_index.back().second;
        auto& coarse_type_freq = coarse_type_freqs[it->second];
        ++coarse_type_freq[p.second];
      }
      for (auto p: extracted_snippets[i]) {
        flow::string::Replace("\"", "", &p.first);
        if (IsBadName(p.first)) {
          continue;
        }
        std::transform(p.first.begin(), p.first.end(), p.first.begin(),
            ::tolower);
        string space_less = p.first;
        flow::string::Replace(" ", "", &space_less);
        const int ontology_id = ontology.LhsNameId(space_less);
        auto it = entity_ids.find(p.first);
        if (it == entity_ids.end()) {
          it = entity_ids.insert({p.first, entity_items.size()}).first;
          entity_items.push_back({p.first, p.second, 0, 0,
                                  ontology.LhsFrequency(ontology_id),
                                  0.0f, {}, {}});
          coarse_type_freqs.push_back({{0, 0, 0, 0}});
        }
        EntityItem& item = entity_items[it->second];
        ++item.snippet_freq;
        if (item.snippet_index.empty() ||
            item.snippet_index.back().first != i) {
          item.snippet_index.push_back({i, 0});
        }
        ++item.snippet_index.back().second;
        auto& coarse_type_freq = coarse_type_freqs[it->second];
        ++coarse_type_freq[p.second];
      }
    }
    #pragma omp parallel for
    for (size_t i = 0; i < entity_items.size(); ++i) {
      Entity::Type best_type = Entity::kPersonType;
      int best_freq = coarse_type_freqs[i][0];
      for (int j = 1; j < 4; ++j) {
        if (coarse_type_freqs[i][j] > best_freq) {
          best_type = static_cast<Entity::Type>(j);
          best_freq = coarse_type_freqs[i][j];
        }
      }
      entity_items[i].coarse_type = best_type;
    }
  }
  end_time = Clock();

  LOG(INFO) << "Filtered entities: " << FilterEntityItems(&entity_items)
      << ", now: " << entity_items.size();

  // LOG(INFO) << EntityItem::JsonArray(entity_items);
  response_stream << ",\"entity_extraction\":{"
      << "\"duration\":" << (end_time - start_time).Value()
      << ",\"entity_items\":" << EntityItem::JsonArray(entity_items) << "}";
  } catch(const Poco::Exception& e) {
    LOG(ERROR) << e.what();
  } catch(const std::exception& e) {
    LOG(ERROR) << e.what();
  } catch (...) {
    LOG(ERROR) << "Unknown exception occured.";
  }
  end_time = Clock();
  response_stream << ",\"duration\":"
      << (end_time - request_start_time).Value() << "}";
}

}  // namespace net
}  // namespace pyt
