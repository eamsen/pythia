// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./full-query-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/Net/HTTPSClientSession.h>
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
  using Poco::Net::HTTPSClientSession;
  using Poco::Net::HTTPRequest;
  using Poco::Net::HTTPResponse;

  const Query query(uri_.getQuery());
  const string& query_text = query.Text("qf");
  const string& query_uri = query.Uri("qf");
  LOG(INFO) << "Full query request: " << query_text << ".";

  vector<string> target_keywords = query_analyser_.TargetKeywords(query_text);
  LOG(INFO) << "Target keywords: " << JsonArray(target_keywords.begin(),
                                                target_keywords.end());

  HTTPSClientSession session(server_.SearchHost());
  HTTPRequest search_request(HTTPRequest::HTTP_GET,
                             server_.SearchBase() + query_uri);
  session.sendRequest(search_request);
  HTTPResponse search_response;
  std::istream& stream = session.receiveResponse(search_response);
  string msg;
  while (stream.good()) {
    string buffer;
    std::getline(stream, buffer);
    msg += buffer;
  }

  Poco::JSON::Parser json_parser;
  Poco::JSON::DefaultHandler json_handler;
  json_parser.setHandler(&json_handler);
  json_parser.parse(msg);
  Poco::Dynamic::Var json_msg = json_handler.result();
  Poco::JSON::Query json_query(json_msg);
  auto items = json_query.findArray("items");

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
