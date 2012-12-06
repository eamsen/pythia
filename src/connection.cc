// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./connection.h"
#include <boost/bind.hpp>
#include <glog/logging.h>

namespace pyt {

Connection::Connection(boost::asio::io_service& service, RequestHandler& handler)
    : socket_(service),
      handler_(handler) {}

boost::asio::ip::tcp::socket& Connection::Socket() {
  return socket_;
}

void Connection::Start() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          boost::bind(&Connection::handleRead, shared_from_this(),
                                    boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
}

void Connection::handleRead(const boost::system::error_code& e, size_t read) {
  if (e) {
    LOG(ERROR) << e;
    return;
  }
  boost::tribool result;
  boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
      request_, buffer_.data(), buffer_.data() + bytes_transferred);
  if (result) {
    reply_ = handler_.Handle(request_);
    boost::asio::async_write(socket_, reply_.ToBuffers(),
                             boost::bind(&Connection::handleWrite,
                             shared_from_this(),
                             boost::asio::placeholders::error));
  } else if (!result) {
    reply_ = Reply::Stock(Reply::kBadRequest);
    boost::asio::async_write(socket_, reply_.ToBuffers(),
                             boost::bind(&Connection::handleWrite,
                             shared_from_this(),
                             boost::asio::placeholders::error));
  } else {
    socket_.ansync_read_some(boost::asio::buffer(buffer_),
                             boost::bind(&Connection::handleRead,
                             shared_from_this(),
                             boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred));
  }
}

void Connection::handleWrite(const boost::system::error_code& e) {
  if (e) {
    LOG(ERROR) << e;
    return;
  }
  boost::system::error_code e;
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, e);
}

}  // namespace pyt

