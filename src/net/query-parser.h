// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_QUERY_PARSER_H_
#define SRC_NET_QUERY_PARSER_H_

#include <string>
#include <unordered_map>

namespace pyt {
namespace net {

class Query {
 public:
  explicit Query(const std::string& query) {
    partition(query);
  }

  bool Empty() const {
    return parts_.size();
  }

  const std::string& operator[](const std::string& key) const {
    static std::string _empty;
    const auto it = parts_.find(key);
    if (it == parts_.end()) {
      return _empty;
    }
    return it->second;
  }

 private:
  void partition(const std::string& query) {
    size_t pos = 0;
    while (pos != std::string::npos) {
      size_t end = query.find("=", pos);
      std::string& value = parts_[query.substr(pos, end - pos)];
      ++end;
      pos = query.find("&", end);
      value = query.substr(end, pos - end);
    }
  }

  std::unordered_map<std::string, std::string> parts_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_QUERY_PARSER_H_
