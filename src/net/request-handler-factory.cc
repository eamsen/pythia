// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./request-handler-factory.h"
#include <glog/logging.h>
#include <Poco/URI.h>
#include <string>
#include <vector>
#include "./document-request-handler.h"
#include "./full-query-request-handler.h"

using std::string;
using std::vector;
using Poco::URI;

namespace pyt {
namespace net {

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest& request) {
  URI uri(request.getURI());
  const string query = uri.getQuery();
  LOG(INFO) << "Request from " << request.clientAddress().toString()
            << ": path(" << uri.getPath()
            << "), query(" << query << ").";
  if (query.find("qf=") != string::npos) {
    // Full query request.
    return new FullQueryRequestHandler(uri);
  }
  // Document request.
  return new DocumentRequestHandler();
}

}  // namespace net
}  // namespace pyt
