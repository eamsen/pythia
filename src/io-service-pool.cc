// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./io-service-pool.h"
#include <glog/logging.h>

namespace pyt {

IoServicePool::IoServicePool(const size_t pool_size)
    : next_(0u) {
  LOG_IF(FATAL, pool_size < 1) << "IO-Service-Pool size is < 1.";
  for (size_t i = 0; i < pool_size; ++i) {
    IoServicePtr io_service_ptr(new boost::asio::io_service);
    WorkPtr work_ptr(new boost::asio::io_service::work(*io_service_ptr));
    services_.push_back(io_service_ptr);
    work_.push_back(work_ptr);
  }
}

void IoServicePool::Run() {}
void IoServicePool::Stop() {}

boost::asio::io_service& IoServicePool::Next() {
  auto service_ptr = services_[next_];
  next_ = next_ + 1 % services_.size();
  return *service_ptr;
}

}  // namespace pyt
