// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_QUERY_PARSER_H_
#define SRC_NET_QUERY_PARSER_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

namespace pyt {
namespace net {

class Query {
 public:
  explicit Query(const std::string& query) {
    partition(query);
  }

  bool Empty() const {
    return word_index_.size();
  }

  const std::vector<std::string>& Words(const std::string& key) const {
    static const std::vector<std::string> _empty = {""};
    const auto it = word_index_.find(key);
    if (it == word_index_.end()) {
      return _empty;
    }
    return it->second;
  }

  std::string Text(const std::string& key) const {
    return join(Words(key), " ");
  }

  std::string Uri(const std::string& key) const {
    return join(Words(key), "+");
  }

 private:
  void partition(const std::string& query) {
    size_t pos = 0;
    while (pos < query.size()) {
      size_t end = query.find("=", pos);
      std::vector<std::string>& words = word_index_[
          query.substr(pos, end - pos)];
      pos = query.find("&", ++end);
      if (query[end] == '"') {
        words = split(query.substr(end + 1, pos - end - 2));
      } else { 
        words = split(query.substr(end, pos - end));
      }
      pos += pos != std::string::npos;
    }
  }

  static std::vector<std::string> split(
      const std::string& content, const std::string& delims = " ") {
    std::vector<std::string> items;
    size_t pos = content.find_first_not_of(delims);
    while (pos != std::string::npos) {
      size_t end = content.find_first_of(delims, pos);
      if (end == std::string::npos) {
        // Last item found.
        items.push_back(content.substr(pos));
      } else {
        // Item found.
        items.push_back(content.substr(pos, end - pos));
      }
      pos = content.find_first_not_of(delims, end);
    }
    return items;
  }

  static std::string join(
      const std::vector<std::string>& words, const std::string& delim) {
    std::string join;
    for (const std::string& w: words) {
      if (join.size()) {
        join += delim;
      }
      join += w;
    }
    return join;
  }

  std::unordered_map<std::string, std::vector<std::string> > word_index_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_QUERY_PARSER_H_
