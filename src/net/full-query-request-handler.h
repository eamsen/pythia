#ifndef SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_
#define SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_

#include <Poco/URI.h>
#include "./request-handler.h"

namespace pyt {
namespace net {

class FullQueryRequestHandler: public RequestHandler {
 public:
  FullQueryRequestHandler(const Poco::URI& uri);
  void Handle(Request* request, Response* response);
 private:
  Poco::URI uri_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_
