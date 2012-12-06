#ifndef SRC_IO_SERVICE_POOL_H
#define SRC_IO_SERVICE_POOL_H

#include <boost/asio.hpp>
#include <vector>

namespace pyt {

class IoServicePool {
 public:
  explicit IoServicePool(const size_t pool_size);
  void Run();
  void Stop();

  boost::asio::io_service& Next();

 private:
  typedef std::shared_ptr<boost::asio::io_service> IoServicePtr;
  typedef std::shared_ptr<boost::asio::io_service::work> WorkPtr;

  std::vector<IoServicePtr> services_;
  std::vector<WorkPtr> work_;

  size_t next_;
};

}  // namespace pyt
#endif  // SRC_IO_SERVICE_POOL_H
