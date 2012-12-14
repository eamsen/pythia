// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_SERVER_H_
#define SRC_NET_SERVER_H_

#include <Poco/Util/ServerApplication.h>
#include <string>

namespace pyt {
namespace net {

class Server: public Poco::Util::ServerApplication {
 public:
  Server(const std::string& name, const std::string& version,
         const std::string& doc_path, const uint16_t port,
         const uint16_t threads, const uint16_t queue_size);
  void Run();
  const std::string& DocumentPath() const;

 private:
  void initialize(Poco::Util::Application& self);
  void uninitialize();
  int main(const std::vector<std::string>& args);

  std::string name_;
  std::string version_;
  std::string doc_path_;
  uint32_t port_;
  uint32_t num_threads_;
  uint16_t queue_size_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_SERVER_H_
