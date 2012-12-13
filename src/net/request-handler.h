// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_REQUEST_HANDLER_H_
#define SRC_NET_REQUEST_HANDLER_H_

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace pyt {
namespace net {

typedef Poco::Net::HTTPServerRequest Request;
typedef Poco::Net::HTTPServerResponse Response;

class RequestHandler: public Poco::Net::HTTPRequestHandler {
 public:
  void handleRequest(Request& request, Response& response) {  // NOLINT
    Handle(&request, &response);
  }

  virtual void Handle(Request* request, Response* response) = 0;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_REQUEST_HANDLER_H_
