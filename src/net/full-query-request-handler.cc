// Copyright 2012 Eugen Sawin <esawin@me73.com>
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
#include "./server.h"
#include "./query-parser.h"
#include "./http-request.h"
#include "../nlp/entity-index.h"
#include "../nlp/named-entity-extractor.h"

using std::string;
using std::vector;
using pyt::nlp::Tagger;
using pyt::nlp::NamedEntityExtractor;
using pyt::nlp::EntityIndex;
using pyt::nlp::Entity;

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
  const Query query(uri_.getQuery());
  const string& query_text = query.Text("qf");
  const string& query_uri = query.Uri("qf");
  LOG(INFO) << "Full query request: " << query_text << ".";

  vector<string> target_keywords = query_analyser_.TargetKeywords(query_text);
  LOG(INFO) << "Target keywords: " << JsonArray(target_keywords.begin(),
                                                target_keywords.end());

  string response_data = HttpsGetRequest(server_.SearchHost() +
                                         server_.SearchBase() + query_uri);

  Poco::JSON::Parser json_parser;
  Poco::JSON::DefaultHandler json_handler;
  json_parser.setHandler(&json_handler);
  json_parser.parse(response_data);
  Poco::Dynamic::Var json_data = json_handler.result();
  Poco::JSON::Query json_query(json_data);
  EntityIndex index;
  NamedEntityExtractor extractor;
  auto items = json_query.findArray("items");
  for (size_t end = items->size(), i = 0; i < end; ++i) {
    const string& url = items->getObject(i)->getValue<string>("link");
    string content = HttpGetRequest(url);
    extractor.Extract(content, &index);
    std::cerr << url << ": " << content.size() << std::endl;
  }
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  std::ostream& response_stream = response->send();
  response_stream << "{\"results\": ";
  items->stringify(response_stream, 0);
  response_stream << ","
      << "\"target_keywords\":"
      << JsonArray(target_keywords.begin(), target_keywords.end()) << ","
      << "\"target_type\": \"target type\","
      << "\"entities\":[";
  size_t num_top = 10;
  std::unordered_set<string> entities;
  while (num_top && index.QueueSize()) {
    Entity entity = index.PopTop();
    if (entities.count(entity.name)) {
      continue;
    }
    --num_top;
    if (entities.size()) {
      response_stream << ",";
    }
    response_stream << "{\"name\":\"" << entity.name
                    << "\",\"score\":" << index.Frequency(entity) << "}";
    entities.insert(entity.name);
  }
  response_stream << "]}";
}

}  // namespace net
}  // namespace pyt
