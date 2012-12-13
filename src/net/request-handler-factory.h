// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_REQUEST_HANDLER_FACTORY_H_
#define SRC_NET_REQUEST_HANDLER_FACTORY_H_

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

namespace pyt {
namespace net {

class RequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory {
 public:
  RequestHandlerFactory() {}

  Poco::Net::HTTPRequestHandler* createRequestHandler(
      const Poco::Net::HTTPServerRequest& request);
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_REQUEST_HANDLER_FACTORY_H_
