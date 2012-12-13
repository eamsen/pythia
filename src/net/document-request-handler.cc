// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./document-request-handler.h"
#include <ostream>

namespace pyt {
namespace net {

void DocumentRequestHandler::Handle(Request* request, Response* response) {
  // using Poco::Util::Application;
  // Application& app = Application::instance();
  response->setChunkedTransferEncoding(true);
  response->setContentType("text/html");
  std::ostream& str = response->send();
  str << "<html><head><title>Pythia</title>"
      << "<body>Pythia</body></html>";
}

}  // namespace net
}  // namespace pyt
