// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./request-handler-factory.h"
#include "./document-request-handler.h"

namespace pyt {
namespace net {

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest& request) {
  if (request.getURI() == "/") {
    return new DocumentRequestHandler();
  }
  return 0;
}

}  // namespace net
}  // namespace pyt
