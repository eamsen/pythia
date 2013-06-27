// Copyright 2013 Eugen Sawin <esawin@me73.com>
#include "./ground-truth-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/DefaultHandler.h>
#include <Poco/JSON/Query.h>
#include <glog/logging.h>
#include <flow/clock.h>
#include <flow/string.h>
#include <flow/stringify.h>
// #include <flow/io/html.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <tuple>
#include <regex>
#include "./server.h"
#include "./query-parser.h"
#include "./http-request.h"
#include "../nlp/entity-index.h"
#include "../nlp/named-entity-extractor.h"
#include "../nlp/edit-distance.h"
#include "../nlp/grammar-tools.h"

using std::string;
using std::vector;
using std::array;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::tuple;
using std::make_tuple;
using std::get;
using std::log2;
using std::max;
using std::min;
using pyt::nlp::Tagger;
using pyt::nlp::NamedEntityExtractor;
using pyt::nlp::EntityIndex;
using pyt::nlp::Entity;
using pyt::nlp::EditDistance;
using pyt::nlp::PrefixEditDistance;
using pyt::nlp::OntologyIndex;
using pyt::nlp::SingularForms;
using flow::time::ThreadClock;
using flow::time::Clock;
using flow::time::ClockDiff;
using flow::io::JsonArray;

namespace pyt {
namespace net {

GroundTruthRequestHandler::GroundTruthRequestHandler(const Poco::URI& uri)
    : server_(static_cast<Server&>(Poco::Util::Application::instance())),
      uri_(uri) {}

void GroundTruthRequestHandler::Handle(Request* request, Response* response) {
  // Prepare clocks.
  Clock start_time;

  // Prepare response stream.
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  std::ostream& response_stream = response->send();

  auto& web_cache = server_.WebCache();
  const string path = uri_.getQuery();
  LOG(INFO) << "Ground truth request: " << path;

  const string& ground_truth = server_.GroundTruth(path);

  const auto duration = Clock() - start_time;
  response_stream << "{\"duration\":" << duration.Value()
      << ",\"ground_truth\":" << ground_truth
      << "}";
}

}  // namespace net
}  // namespace pyt
