// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./request-handler-factory.h"
#include <string>
#include <vector>
#include <glog/logging.h>
#include <Poco/URI.h>
#include "./document-request-handler.h"

using std::string;
using std::vector;
using Poco::URI;

namespace pyt {
namespace net {

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest& request) {
  URI uri(request.getURI());
  // vector<string> segments;
  // uri.getPathSegments(segments);
  LOG(INFO) << "Request from " << request.clientAddress().toString()
            << ": path(" << uri.getPath()
            << "), query(" << uri.getQuery() << ").";
  if (uri.getQuery().empty()) {
    return new DocumentRequestHandler();
  }
  LOG(WARNING) << "Unknown request type for URI " << uri.toString() << ".";
  google::FlushLogFiles(google::INFO);
  return 0;
}

}  // namespace net
}  // namespace pyt
