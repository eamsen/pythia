#ifndef SRC_NET_DOCUMENT_REQUEST_HANDLER_H_
#define SRC_NET_DOCUMENT_REQUEST_HANDLER_H_

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Exception.h>
#include <Poco/ThreadPool.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <string>
#include <vector>
#include <ostream>

namespace pyt {
namespace net {

class DocumentRequestHandler: public Poco::Net::HTTPRequestHandler {
 public:
  void handleRequest(Poco::Net::HTTPServerRequest& request,  // NOLINT
                     Poco::Net::HTTPServerResponse& response);  // NOLINT
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_DOCUMENT_REQUEST_HANDLER_H_
