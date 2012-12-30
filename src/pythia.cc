// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <vector>
#include <string>
#include "./net/server.h"

using std::vector;
using std::string;

DEFINE_int32(port, 55073, "Server listen port.");
DEFINE_int32(threads, 32, "Max number of threads.");
DEFINE_int32(queue_size, 64, "Max number of queued connections.");
DEFINE_string(doc_path, "www", "Web documents directory.");

static const char* kUsage = "Usage: ./pythia";
static const char* kName = "pythia.me73.com:80";
static const char* kVersion = "Pythia/0.1";

int main(int argc, char* argv[]) {
  google::SetUsageMessage(kUsage);
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  vector<string> args(&argv[1], &argv[argc]);
  LOG(INFO) << "Started.";
  pyt::net::Server server(kName, kVersion, FLAGS_doc_path, FLAGS_port,
                          FLAGS_threads, FLAGS_queue_size);
  server.Run();
  LOG(INFO) << "Shut down.";
}
