// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_QUERY_ANALYSER_H_
#define SRC_NLP_QUERY_ANALYSER_H_

#include <string>
#include <vector>

namespace pyt {
namespace nlp {

class Tagger;

class QueryAnalyser {
 public:
  explicit QueryAnalyser(const Tagger& tagger);
  std::vector<std::string> TargetKeywords(const std::string& query) const;

 private:
  const Tagger& tagger_;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_QUERY_ANALYSER_H_
