// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_CONNECTION_H
#define SRC_CONNECTION_H

#include <array>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "./reply.h"
#include "./request.h"
#include "./request-parser.h"
#include "./request-handler.h"

namespace pyt {

class Connection : public boost::enable_shared_from_this<Connection> {
 public:
  static const size_t kBufferSize = 8192;

  explicit Connection(boost::asio::io_service& service, RequestHandler& handler);
  boost::asio::ip::tcp::socket& Socket();

  void Start();
 private:
  void handleRead(const boost::system::error_code& e, const size_t read);
  void handleWrite(const boost::system::error_code& e);

  boost::asio::ip::tcp::socket socket_;
  RequestHandler& handler_;
  boost::array<char, kBufferSize> buffer_;
  Request request_;
  RequestParser request_parser_;
  Reply reply_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

}  // namespace pyt
#endif  // SRC_CONNECTION_H
