// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_EDIT_DISTANCE_H_
#define SRC_NLP_EDIT_DISTANCE_H_

#include <string>

namespace pyt {
namespace nlp {

int EditDistance(const std::string& word1, const std::string& word2);
int PrefixEditDistance(const std::string& prefix, const std::string& word);

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_EDIT_DISTANCE_H_
