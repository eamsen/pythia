// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_DOCUMENT_REQUEST_HANDLER_H_
#define SRC_NET_DOCUMENT_REQUEST_HANDLER_H_

#include "./request-handler.h"

namespace pyt {
namespace net {

class DocumentRequestHandler: public RequestHandler {
 public:
  void Handle(Request* request, Response* response);
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_DOCUMENT_REQUEST_HANDLER_H_
