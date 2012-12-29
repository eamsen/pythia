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
#include "./server.h"
#include "./query-parser.h"
#include "./http-request.h"

using std::string;
using std::vector;
using pyt::nlp::Tagger;

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

  string msg = HttpsGetRequest(server_.SearchHost() + server_.SearchBase() +
                               query_uri);

  Poco::JSON::Parser json_parser;
  Poco::JSON::DefaultHandler json_handler;
  json_parser.setHandler(&json_handler);
  json_parser.parse(msg);
  Poco::Dynamic::Var json_msg = json_handler.result();
  Poco::JSON::Query json_query(json_msg);
  auto items = json_query.findArray("items");
  for (size_t end = items->size(), i = 0; i < end; ++i) {
    std::cerr << items->getObject(i)->getValue<string>("link") << std::endl;
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
      << "\"entities\": [[\"entity a\", 40], [\"entity b\", 20]]}";
  // google::FlushLogFiles(google::INFO);
}

}  // namespace net
}  // namespace pyt
