// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./server.h"
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

using std::string;
using std::vector;
using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;

namespace pyt {

class DocumentRequestHandler: public HTTPRequestHandler {
 public:
  void handleRequest(HTTPServerRequest& request,  // NOLINT
                     HTTPServerResponse& response) {  // NOLINT
    using Poco::Util::Application;
    // Application& app = Application::instance();
    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");
    std::ostream& str = response.send();
    str << "<html><head><title>Pythia</title>"
        << "<body>Pythia</body></html>";
  }
};

class RequestHandlerFactory: public HTTPRequestHandlerFactory {
 public:
  RequestHandlerFactory() {}

  HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) {
    if (request.getURI() == "/") {
      return new DocumentRequestHandler();
    }
    return 0;
  }
};

class Application: public ServerApplication {
 public:
  explicit Application(uint16_t port)
      : port_(port) {}
 private:
  void initialize(Application& self) {  // NOLINT
    ServerApplication::initialize(self);
  }

  void uninitialize() {
    ServerApplication::uninitialize();
  }

  int main(const vector<string>& args) {
    ServerSocket socket(port_);
    HTTPServer server(new RequestHandlerFactory(), socket,
                      new HTTPServerParams());
    server.start();
    waitForTerminationRequest();
    server.stop();
    return Application::EXIT_OK;
  }

  uint16_t port_;
};

Server::Server(const string& www, const uint16_t port, const uint16_t threads)
    : www_path_(www),
      port_(port),
      num_threads_(threads),
      app_(new Application(port_)) {}

void Server::Run() {
  int argc = 1;
  const char* argv[] = {"pythia"};
  app_->run(argc, const_cast<char**>(argv));
}

}  // namespace pyt

