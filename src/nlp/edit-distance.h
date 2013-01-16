// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_EDIT_DISTANCE_H_
#define SRC_NLP_EDIT_DISTANCE_H_

#include <string>
#include <vector>
#include <limits>

namespace pyt {
namespace nlp {

int EditDistance(const std::string& word1, const std::string& word2) {
  auto Dist = [](int r, int d, int i, bool eq) {
    return std::min(std::min(d, i) + 1, r + !eq);
  };

  const size_t size1 = word1.size();
  const size_t size2 = word2.size();
  std::vector<int> dists(size1 + 1, 0);
  for (size_t i = 1; i < size1 + 1; ++i) {
    dists[i] = i;
  }
  std::vector<int> new_dists(size1 + 1, 0);
  for (size_t w2 = 0; w2 < size2; ++w2) {
    new_dists[0] = dists[0] + 1;
    for (size_t w1 = 0; w1 < size1; ++w1) {
      new_dists[w1 + 1] = Dist(dists[w1], dists[w1 + 1], new_dists[w1],
                               word1[w1] == word2[w2]);
    }
    dists.swap(new_dists);
  }
  return dists.back();
}

int PrefixEditDistance(const std::string& prefix, const std::string& word) {
  auto Dist = [](int r, int d, int i, bool eq) {
    return std::min(std::min(d, i) + 1, r + !eq);
  };

  const size_t size1 = word.size();
  const size_t size2 = prefix.size();
  std::vector<int> dists(size1 + 1, 0);
  for (size_t i = 1; i < size1 + 1; ++i) {
    dists[i] = i;
  }
  std::vector<int> new_dists(size1 + 1, 0);
  for (size_t w2 = 0; w2 < size2; ++w2) {
    new_dists[0] = dists[0] + 1;
    for (size_t w1 = 0; w1 < size1; ++w1) {
      new_dists[w1 + 1] = Dist(dists[w1], dists[w1 + 1], new_dists[w1],
                               word[w1] == prefix[w2]);
    }
    dists.swap(new_dists);
  }
  std::sort(dists.begin(), dists.end());
  return dists[0];
}

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_EDIT_DISTANCE_H_
