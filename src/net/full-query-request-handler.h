// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_
#define SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_

#include <Poco/URI.h>
#include "./request-handler.h"
#include "../nlp/query-analyser.h"

namespace pyt {
namespace net {

class Server;

class FullQueryRequestHandler: public RequestHandler {
 public:

  explicit FullQueryRequestHandler(const Poco::URI& uri);
  void Handle(Request* request, Response* response);
 private:
  Server& server_;
  Poco::URI uri_;
  pyt::nlp::QueryAnalyser query_analyser_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_
