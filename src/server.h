// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_SERVER_H
#define SRC_SERVER_H

#include <string>
#include "./io-service-pool.h"
#include "./connection.h"
#include "./request-handler.h"

namespace pyt {

class Server {
 public:
  Server(const std::string& docs_dir, const int port, const int num_threads);
  void Run();

 private:
  void startAccept();
  void handleAccept(const boost::system::error_code& e);

  std::string docs_dir_;
  int port_;
  int num_threads_;
  IoServicePool service_pool_;
  boost::asio::ip::tcp::acceptor acceptor_;
  ConnectionPtr connection_ptr_;
  RequestHandler request_handler_;
};

}  // namspace pyt
#endif  // SRC_SERVER_H
