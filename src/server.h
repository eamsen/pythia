// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_SERVER_H
#define SRC_SERVER_H

#include <string>

namespace pyt {

class Application;

class Server {
 public:
  Server(const std::string& www, const uint16_t port, const uint16_t threads);
  void Run();
 private:
  std::string www_path_;
  uint32_t port_;
  uint32_t num_threads_;

  Application* app_;
};

}  // namespace pyt
#endif  // SRC_SERVER_H
