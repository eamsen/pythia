// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./full-query-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <glog/logging.h>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include "./server.h"
#include "./query-parser.h"

using std::string;
using std::vector;
using pyt::nlp::Tagger;

namespace pyt {
namespace net {

template<typename It>
string JsonArray(It begin, It end) {
  std::stringstream ss;
  ss << "[";
  It it = begin;
  while (it != end) {
    if (it != begin) {
      ss << ",";
    }
    ss << "\"" << *it << "\"";
    ++it;
  }
  ss << "]";
  return ss.str();
}

vector<string> TargetKeywords(const Tagger& tagger, const string& query) {
  auto IsNN = [](const int label) {
    return label == Tagger::kPosNN || label == Tagger::kPosNNP ||
           label == Tagger::kPosNNS || label == Tagger::kPosNNPS;
  };

  auto Singular = [](const string& word, const int label) {
    if ((label == Tagger::kPosNNS || label == Tagger::kPosNNPS) &&
        word[word.size() - 1] == 's') {
      return word.substr(0, word.size());
    }
    return word;
  };

  vector<string> keywords;
  vector<Tagger::Tag> tags = tagger.Tags(query);
  for (auto beg = tags.cbegin(), end = tags.cend(), it = beg; it != end; ++it) {
    const int label = it->label;
    if (IsNN(label)) {
      if (keywords.empty() ||
          (it - beg > 1 && (it - 1)->label == Tagger::kPosCC &&
           IsNN((it - 2)->label))) {
        // Simple NN or NN conjunction.
        keywords.push_back(Singular(query.substr(it->offset.begin,
                                                 it->offset.end), label));
      } else if (it != beg && IsNN((it - 1)->label)) {
        // Multi-word NN.
        keywords.back() += " " + Singular(query.substr(it->offset.begin,
                                                       it->offset.end), label);
      } else if (keywords.size()) {
        // Found all relevant target keywords.
        break;
      }
    } else if (label == Tagger::kPosPOS) {
      // Possesive ending, target NN is following.
      keywords.pop_back();
    }
  }
  return keywords;
}

FullQueryRequestHandler::FullQueryRequestHandler(const Poco::URI& uri)
    : uri_(uri) {}

void FullQueryRequestHandler::Handle(Request* request, Response* response) {
  using Poco::Net::HTTPSClientSession;
  using Poco::Net::HTTPRequest;
  using Poco::Net::HTTPResponse;

  Server& server = static_cast<Server&>(Poco::Util::Application::instance());
  const Query query(uri_.getQuery());
  LOG(INFO) << "Full query request: " << query["qf"] << ".";

  string decoded_query;
  Poco::URI::decode(query["qf"], decoded_query);
  vector<string> target_keywords = TargetKeywords(server.Tagger(),
                                                  decoded_query);
  LOG(INFO) << "Target keywords: " << JsonArray(target_keywords.begin(),
                                                target_keywords.end());

  HTTPSClientSession session(server.SearchHost());
  HTTPRequest search_request(HTTPRequest::HTTP_GET,
                             server.SearchBase() + query["qf"]);
  session.sendRequest(search_request);
  HTTPResponse search_response;
  std::istream& stream = session.receiveResponse(search_response);
  string msg;
  while (stream.good()) {
    string buffer;
    std::getline(stream, buffer);
    msg += buffer;
  }
  // LOG(INFO) << msg;
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/plain");
  response->send() << "{\"results\": " << msg << ","
      << "\"target_keywords\":"
      << JsonArray(target_keywords.begin(), target_keywords.end()) << ","
      << "\"target_type\": \"target type\","
      << "\"entities\": [[\"entity a\", 40], [\"entity b\", 20]]}";
  // google::FlushLogFiles(google::INFO);
}

}  // namespace net
}  // namespace pyt
