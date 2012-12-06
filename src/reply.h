#ifndef SRC_REPLY_H
#define SRC_REPLY_H

namespace pyt {

struct Reply {
  static Reply Stock(Status status);

  enum Status {
    kOk = 200,
    kCreated = 201,
    kAccepted = 202,
    kNoContent = 204,
    kMultipleChoices = 300,
    kMovedPermanently = 301,
    kMovedTemporarily = 302,
    kNotModified = 304,
    kBadRequest = 400,
    kUnauthorized = 401,
    kForbidden = 403,
    kNotFound = 404,
    kInternalServerError = 500,
    kNotImplemented = 501,
    kBadGateway = 502,
    kServiceUnavailable = 503
  };

  std::vector<Header> headers;
  std::string content;
  std::vector<boost::asio::const_buffer> ToBuffers();
};
}  // namespace pyt
#endif  // SRC_REPLY_H
