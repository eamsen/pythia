// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_GROUND_TRUTH_REQUEST_HANDLER_H_
#define SRC_NET_GROUND_TRUTH_REQUEST_HANDLER_H_

#include <Poco/URI.h>
#include "./request-handler.h"
#include "../nlp/query-analyser.h"

namespace pyt {
namespace net {

class Server;

class GroundTruthRequestHandler: public RequestHandler {
 public:
  explicit GroundTruthRequestHandler(const Poco::URI& uri);
  void Handle(Request* request, Response* response);

 private:
  Server& server_;
  Poco::URI uri_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_GROUND_TRUTH_REQUEST_HANDLER_H_
