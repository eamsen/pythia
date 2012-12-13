// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include "net/server.h"

using std::vector;
using std::string;

DEFINE_int32(port, 8080, "Server listen port.");
DEFINE_int32(threads, 4, "Number of threads.");
DEFINE_string(www, "www", "Web documents directory.");

const char* kUsage = "Usage: ./pythia";

namespace pyt {

}  // namespace pyt

int main(int argc, char* argv[]) {
  google::SetUsageMessage(kUsage);
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  vector<string> args(&argv[1], &argv[argc]);
  LOG(INFO) << "Starting Pythia.";
  pyt::net::Server server(FLAGS_www, FLAGS_port, FLAGS_threads);
  server.Run();
}
