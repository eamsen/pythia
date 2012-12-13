// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./document-request-handler.h"

namespace pyt {
namespace net {


  void DocumentRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,  // NOLINT
                     Poco::Net::HTTPServerResponse& response) {  // NOLINT
    using Poco::Util::Application;
    // Application& app = Application::instance();
    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");
    std::ostream& str = response.send();
    str << "<html><head><title>Pythia</title>"
        << "<body>Pythia</body></html>";
  }
}
}
