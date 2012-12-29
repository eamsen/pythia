// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./http-request.h"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>

using std::string;
using Poco::Net::HTTPSClientSession;
using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::URI;

namespace pyt {
namespace net {

template<class SessionType>
void SendRequest(const URI& uri, SessionType* session) {
  HTTPRequest request(HTTPRequest::HTTP_GET, uri.getPathEtc());
  session->sendRequest(request);
}

template<class SessionType>
string ReceiveResponse(SessionType* session) {
  HTTPResponse response;
  std::istream& stream = session->receiveResponse(response);
  std::streamsize size = response.getContentLength();
  string data;
  if (size == HTTPResponse::UNKNOWN_CONTENT_LENGTH) {
    // Content-length header field not set.
    while (stream.good()) {
      string buffer;
      std::getline(stream, buffer);
      data += buffer;
    }
  } else {
    // Content length is known.
    data.resize(size);
    stream.read(&data[0], size);
  }
  return data;
}

string HttpGetRequest(const string& url) {
  URI uri(url);
  HTTPClientSession session(uri.getHost());
  SendRequest(uri, &session);
  return ReceiveResponse(&session);
}

string HttpsGetRequest(const string& url) {
  URI uri(url);
  HTTPSClientSession session(uri.getHost());
  SendRequest(uri, &session);
  return ReceiveResponse(&session);
}

}  // namespace net
}  // namespace pyt
