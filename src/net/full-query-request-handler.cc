// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./full-query-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <glog/logging.h>
#include <ostream>
#include <string>
#include <vector>
#include "./server.h"
#include "./query-parser.h"

using std::string;
using std::vector;

namespace pyt {
namespace net {

FullQueryRequestHandler::FullQueryRequestHandler(const Poco::URI& uri)
    : uri_(uri) {}

void FullQueryRequestHandler::Handle(Request* request, Response* response) {
  using Poco::Net::HTTPSClientSession;
  using Poco::Net::HTTPRequest;
  using Poco::Net::HTTPResponse;

  Server& server = dynamic_cast<Server&>(Poco::Util::Application::instance());
  const Query query(uri_.getQuery());
  LOG(INFO) << "Full query request: " << query["qf"] << ".";
  
  HTTPSClientSession session(server.SearchHost());
  HTTPRequest search_request(HTTPRequest::HTTP_GET,
                             server.SearchBase() + query["qf"]);
  session.sendRequest(search_request);
  HTTPResponse search_response;
  std::istream& stream = session.receiveResponse(search_response);
  LOG(INFO) << search_response.getStatus() << ".";
  LOG(INFO) << search_response.getContentLength() << ".";
  string msg;
  while (stream.good()) {
    string buffer;
    std::getline(stream, buffer);
    msg += buffer;
  }
  // LOG(INFO) << msg;
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  response->send() << "{\"results\": " << msg << "," 
                   << "\"target_keywords\": [\"keyword\"],"
                   << "\"target_type\": \"target type\","
                   << "\"entities\": [[\"entity a\", 400], [\"entity b\", 200]]}";
  // google::FlushLogFiles(google::INFO);
}

}  // namespace net
}  // namespace pyt
