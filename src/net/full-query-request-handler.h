#ifndef SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_
#define SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_

#include "./request-handler.h"

namespace pyt {
namespace net {

class FullQueryRequestHandler: public RequestHandler {
 public:
  void Handle(Request* request, Response* response);
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_FULL_QUERY_REQUEST_HANDLER_H_
