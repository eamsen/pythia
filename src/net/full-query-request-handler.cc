// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./full-query-request-handler.h"
#include <Poco/Exception.h>
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

void FullQueryRequestHandler::Handle(Request* request, Response* response) {
  Server& server = dynamic_cast<Server&>(Poco::Util::Application::instance());
  const string& uri = request->getURI();
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  response->send() << "{\"results\": [],"
                   << "\"target_keywords\": [\"keyword\"],"
                   << "\"target_type\": \"target type\","
                   << "\"entities\": [[\"entity a\", 400], [\"entity b\", 200]]}";
}

}  // namespace net
}  // namespace pyt
