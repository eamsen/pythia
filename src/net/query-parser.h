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
    return text_parts_.size();
  }

  const std::string& Text(const std::string& key) const {
    static std::string _empty;
    const auto it = text_parts_.find(key);
    if (it == text_parts_.end()) {
      return _empty;
    }
    return it->second;
  }

  const std::string& Uri(const std::string& key) const {
    static std::string _empty;
    const auto it = uri_parts_.find(key);
    if (it == uri_parts_.end()) {
      return _empty;
    }
    return it->second;
  }

 private:
  void partition(const std::string& query) {
    size_t pos = 0;
    while (pos != std::string::npos) {
      size_t end = query.find("=", pos);
      std::string& text_value = text_parts_[query.substr(pos, end - pos)];
      std::string& uri_value = uri_parts_[query.substr(pos, end - pos)];
      ++end;
      pos = query.find("&", end);
      text_value = query.substr(end, pos - end);
      uri_value = query.substr(end, pos - end);
      std::replace(text_value.begin(), text_value.end(), '+', ' ');
    }
  }

  std::unordered_map<std::string, std::string> text_parts_;
  std::unordered_map<std::string, std::string> uri_parts_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_QUERY_PARSER_H_
