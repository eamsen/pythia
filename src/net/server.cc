// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_SERVER_APPLICATION_H_
#define SRC_NET_SERVER_APPLICATION_H_

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

Server::Server(const string& name, const string& version,
               const string& doc_path, const uint16_t port,
               const uint16_t threads, const uint16_t queue_size)
    : name_(name),
      version_(version),
      doc_path_(doc_path),
      port_(port),
      num_threads_(threads),
      queue_size_(queue_size) {}

void Server::Run() {
  vector<string> args = {"pythia"};
  run(args);
}

const string& Server::DocumentPath() const {
  return doc_path_;
}

void Server::initialize(Application& self) {  // NOLINT
  ServerApplication::initialize(self);
}

void Server::uninitialize() {
  ServerApplication::uninitialize();
}

int Server::main(const vector<string>& args) {
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

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_SERVER_APPLICATION_H_
