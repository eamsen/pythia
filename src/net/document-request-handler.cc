// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./document-request-handler.h"
#include <Poco/Exception.h>
#include <glog/logging.h>
#include <ostream>
#include <string>
#include <vector>
#include "./server.h"

using std::string;
using std::vector;

namespace pyt {
namespace net {

class ContentType {
 public:
  ContentType(const string& doc) {
    const auto type = GetType(doc);
    type_ = type.first;
    subtype_ = type.second;
  }

  const string& Type() const {
    return types_[type_];
  }

  const string& Subtype() const {
    return subtypes_[subtype_];
  }

  string FullType() const {
    return Type() + "/" + Subtype(); 
  }

  static std::pair<size_t, size_t> GetType(const string& doc) {
    const size_t pos = doc.rfind(".");
    if (pos == string::npos) {
      // Unknown type.
      return {2, 5};
    }
    const string suffix = doc.substr(pos + 1);
    if (suffix == "html") {
      return {2, 2};
    } else if (suffix == "css") {
      return {2, 0};
    } else if (suffix == "ico") {
      return {1, 7};
    } else if (suffix == "js") {
      return {0, 3};
    } else if (suffix == "jpg") {
      return {1, 8};
    } else if (suffix == "exe") {
      return {1, 9};
    } else if (suffix == "pdf") {
      return {1, 10};
    }
    // Default type.
    return {2, 5};
  }

 private:
  static const vector<string> types_; 
  static const vector<string> subtypes_;

  size_t type_;
  size_t subtype_;
};

const vector<string> ContentType::types_ =
// 0,           1,       2,      3,
{"application", "image", "text", "unknown"};

const vector<string> ContentType::subtypes_ =
// 0,   1,     2,      3,            4,      5,       6,     7,        8,
{"css", "csv", "html", "javascript", "json", "plain", "xml", "x-icon", "jpeg",
// 9,             10,
  "octet-stream", "pdf"};

void DocumentRequestHandler::Handle(Request* request, Response* response) {
  Server& server = dynamic_cast<Server&>(Poco::Util::Application::instance());
  const string& uri = request->getURI();
  ContentType type(uri);
  DLOG(INFO) << "Content type for " << uri  
             << ": " << type.FullType() << ".";
  response->setChunkedTransferEncoding(true);
  try {
    response->sendFile(server.DocumentPath() + "/" + uri, type.FullType());
  } catch (const Poco::FileException& e) {
    LOG(WARNING) << e.name() << ": " << uri;
    response->setContentType("text/plain");
    response->send() << "File not found.";
  }
}

}  // namespace net
}  // namespace pyt
