// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./server.h"
#include <sstream>
#include <glog/logging.h>
#include <boost/bind.hpp>

using std::string;

namespace pyt {

Server::Server(const string& docs_dir, const int port, const int num_threads)
    : docs_dir_(docs_dir),
      port_(port),
      num_threads_(num_threads),
      service_pool_(num_threads),
      acceptor_(service_pool_.Next()) {
  boost::asio::ip::tcp::resolver resolver(acceptor_.get_io_service());
  std::ostringstream ss;
  ss << port;
  boost::asio::ip::tcp::resolver::query query("", ss.str());
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  startAccept();
}

void Server::Run() {
  LOG(INFO) << "Listening at " << port_ << ".";
  LOG(INFO) << "Using " << num_threads_ << " threads.";
  LOG(INFO) << "Serving docs from " << docs_dir_ << ".";
  service_pool_.Run();
}

void Server::startAccept() {
  connection_ptr_.reset(new Connection(service_pool_.Next(), request_handler_));
  acceptor_.async_accept(connection_ptr_->Socket(),
                         boost::bind(&Server::handleAccept, this,
                                   boost::asio::placeholders::error));
}

void Server::handleAccept(const boost::system::error_code& e) {
  if (!e) {
    connection_ptr_->Start();
  }
  startAccept();
}

}  // namespace pyt
