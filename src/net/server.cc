// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./server.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/ThreadPool.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <string>
#include <vector>
#include <ostream>
#include "./request-handler-factory.h"

using std::string;
using std::vector;
using Poco::Net::ServerSocket;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;

namespace pyt {
namespace net {

class Application: public ServerApplication {
 public:
  Application(const string& name, const string& version, const uint16_t port,
              const uint16_t threads, const uint16_t queue_size)
      : name_(name),
        version_(version),
        port_(port),
        num_threads_(threads),
        queue_size_(queue_size) {}

 private:
  void initialize(Application& self) {  // NOLINT
    ServerApplication::initialize(self);
  }

  void uninitialize() {
    ServerApplication::uninitialize();
  }

  int main(const vector<string>& args) {
    ServerSocket socket(port_);
    HTTPServerParams* params = new HTTPServerParams();
    params->setServerName(name_);
    params->setSoftwareVersion(version_);
    params->setMaxQueued(queue_size_);
    params->setMaxThreads(num_threads_);
    HTTPServer server(new RequestHandlerFactory(), socket, params);
    server.start();
    waitForTerminationRequest();
    server.stop();
    return Application::EXIT_OK;
  }

  string name_;
  string version_;
  uint16_t port_;
  uint16_t num_threads_;
  uint16_t queue_size_;
};

Server::Server(const string& name, const string& version, const string& www,
               const uint16_t port, const uint16_t threads,
               const uint16_t queue_size)
    : name_(name),
      version_(version),
      www_path_(www),
      port_(port),
      num_threads_(threads),
      queue_size_(queue_size),
      app_(new Application(name, version, port, threads, queue_size)) {}

void Server::Run() {
  int argc = 1;
  const char* argv[] = {"pythia"};
  app_->run(argc, const_cast<char**>(argv));
}

}  // namespace net
}  // namespace pyt
