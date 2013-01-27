// Copyright 2013 Eugen Sawin <esawin@me73.com>
#include "./grammar-tools.h"
#include <glog/logging.h>
#include <unordered_map>

using std::vector;
using std::unordered_map;
using std::string;

namespace pyt {
namespace nlp {

vector<string> SingularForms(const string& noun) {
  static const unordered_map<string, string> special =
      {{"men", "man"}, {"women", "woman"}, {"people", "person"},
       {"children", "child"}, {"brethren", "brother"}};

  DLOG_IF(WARNING, !IsLowerCase(noun)) << "Lower case string expected.";

  const size_t size = noun.size();
  vector<string> sings = {noun};
  const auto it = special.find(noun);
  if (it != special.end()) {
    sings.push_back(it->second);
  }
  if (size && noun[size-1] == 's') {
    // Remove trailing 's'.
    sings.push_back(noun.substr(0, size-1));
    if (size > 3 && noun[size-2] == 'e') {
      // Trailing 'es' may indicate ending with a whistling sound.
      if (noun[size-3] == 's' || noun[size-3] == 'x' || noun[size-3] == 'z') {
        sings.push_back(noun.substr(0, size-2));
      } else if (noun[size-3] == 'h' &&
                 (noun[size-4] == 's' || noun[size-4] == 'c')) {
        sings.push_back(noun.substr(0, size-2));
      }
      if (noun[size-3] == 'v') {
        // Trailing 'ves' may indicate ending with a f or fe.
        sings.push_back(noun.substr(0, size-3) + "f");
        sings.push_back(noun.substr(0, size-3) + "fe");
      } else if (IsConsonant(noun[size-4])) {
        if (noun[size-3] == 'i') {
          // Trailing 'es' may also indicate ending with a consonant + y, where
          // the y is replaced by i.
          sings.push_back(noun.substr(0, size-3) + "y");
        } else if (noun[size-3] == 'o') {
          // Trailing 'es' may also indicate ending with a consonant + o.
          sings.push_back(noun.substr(0, size-2));
        }
      }
    }
  }
  return sings;
}

// a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z.
bool IsConsonant(const char c) {
  static const std::vector<char> is = {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1,
                                       0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1};
  // TODO(esawin): How to handle 'y'.
  DLOG_IF(WARNING, c < 'a' || c > 'z') << "Lower case characters expected.";
  return c >= 'a' && c <= 'z' && is[c - 'a'];
}

bool IsVowel(const char c) {
  return !IsConsonant(c);
}

bool IsLowerCase(const std::string& word) {
  for (const char c: word) {
    if (c < 'a' || c > 'z') {
      return false;
    }
  }
  return true;
}

}  // namespace nlp
}  // namespace pyt
