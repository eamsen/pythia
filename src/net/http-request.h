// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_HTTP_REQUEST_H_
#define SRC_NET_HTTP_REQUEST_H_

#include <string>

namespace pyt {
namespace net {

std::string HttpGetRequest(const std::string& url, const int64_t timeout);
std::string HttpsGetRequest(const std::string& url, const int64_t timeout);

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_HTTP_REQUEST_H_
