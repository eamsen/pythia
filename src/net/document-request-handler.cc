// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./document-request-handler.h"
#include <ostream>
#include "./server.h"

namespace pyt {
namespace net {

void DocumentRequestHandler::Handle(Request* request, Response* response) {
  Server& server = dynamic_cast<Server&>(Poco::Util::Application::instance());

  response->setChunkedTransferEncoding(true);
  response->setContentType("text/html");
  std::ostream& str = response->send();
  str << "<html><head><title>Pythia</title>"
      << "<body>Pythia</body></html>";
}

}  // namespace net
}  // namespace pyt
