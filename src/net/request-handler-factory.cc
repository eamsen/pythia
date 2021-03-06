// Copyright 2012, 2013 Eugen Sawin <esawin@me73.com>
#include "./request-handler-factory.h"
#include <glog/logging.h>
#include <Poco/URI.h>
#include <string>
#include <vector>
#include "./document-request-handler.h"
#include "./full-query-request-handler.h"
#include "./type-info-request-handler.h"
#include "./ground-truth-request-handler.h"

using std::string;
using std::vector;
using Poco::URI;

namespace pyt {
namespace net {

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest& request) {
  URI uri(request.getURI());
  const string query = uri.getQuery();
  const string path = uri.getPath();
  DLOG(INFO) << "Request from " << request.clientAddress().toString()
      << "; path: " << path 
      << "; query: " << query;
  if (query.find("qf=") != string::npos) {
    // Full query request.
    return new FullQueryRequestHandler(uri);
  }
  if (query.find("ti=") != string::npos) {
    // YAGO and Freebase type identification request.
    return new TypeInfoRequestHandler(uri);
  }
  if (path.find("/ground-truth/") != string::npos) {
    // Ground truth request.
    return new GroundTruthRequestHandler(uri);
  }
  // Document request.
  return new DocumentRequestHandler();
}

}  // namespace net
}  // namespace pyt
