// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_GRAMMAR_TOOLS_H_
#define SRC_NLP_GRAMMAR_TOOLS_H_

#include <vector>
#include <string>

namespace pyt {
namespace nlp {

std::vector<std::string> SingularForms(const std::string& noun);
bool IsConsonant(const char c);
bool IsVowel(const char c);
bool IsLowerCase(const std::string& word);

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_GRAMMAR_TOOLS_H_
