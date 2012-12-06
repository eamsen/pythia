// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <vector>
#include <string>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <boost/asio.hpp>
#include "./server.h"

using std::vector;
using std::string;

DEFINE_int32(port, 8080, "Server listen port.");
DEFINE_int32(threads, 4, "Number of threads.");
DEFINE_string(www, "www", "Web documents directory.");

namespace pyt {

}  // namespace pyt

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  vector<string> args(&argv[1], &argv[argc]);
  LOG(INFO) << "Starting Pythia.";
  pyt::Server server(FLAGS_www, FLAGS_port, FLAGS_threads);
  server.Run();
}
