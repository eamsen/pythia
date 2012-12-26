// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./document-request-handler.h"
#include <Poco/Exception.h>
#include <Poco/URI.h>
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
  explicit ContentType(const string& doc) {
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
      return {2, 2};
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
      return {0, 9};
    } else if (suffix == "pdf") {
      return {0, 10};
    } else if (suffix == "woff") {
      return {0, 12};
    } else if (suffix == "ttf") {
      return {0, 11};
    } else if (suffix == "svg") {
      return {1, 14};
    } else if (suffix == "eot") {
      return {0, 13};
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
//  9,              10,    11,           12,            13,
    "octet-stream", "pdf", "x-font-ttf", "x-font-woff", "vnd.ms-fontobject",
//  14
    "svg+xml"};

void DocumentRequestHandler::Handle(Request* request, Response* response) {
  Server& server = static_cast<Server&>(Poco::Util::Application::instance());
  const Poco::URI uri(request->getURI());
  ContentType type(uri.getPath());
  DLOG(INFO) << "Content type for " << uri.getPath()
             << ": " << type.FullType() << ".";
  response->setChunkedTransferEncoding(true);
  try {
    string file_path = server.DocumentPath() + "/";
    if (uri.getPath().empty() || uri.getPath() == "/") {
      file_path += "index.html";
    } else {
      file_path += uri.getPath();
    }
    response->sendFile(file_path, type.FullType());
  } catch(const Poco::FileException& e) {
    LOG(WARNING) << e.name() << ": " << uri.getPath();
    response->setContentType("text/plain");
    response->send() << "File not found.";
  }
}

}  // namespace net
}  // namespace pyt
