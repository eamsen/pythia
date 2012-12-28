// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./query-analyser.h"
#include "./tagger.h"

using std::string;
using std::vector;

namespace pyt {
namespace nlp {

QueryAnalyser::QueryAnalyser(const Tagger& tagger)
    : tagger_(tagger) {}

vector<string> QueryAnalyser::TargetKeywords(const string& query) const {
  auto IsNN = [](const int label) {
    return label == Tagger::kPosNN || label == Tagger::kPosNNP ||
           label == Tagger::kPosNNS || label == Tagger::kPosNNPS;
  };

  auto Singular = [](const string& word, const int label) {
    if ((label == Tagger::kPosNNS || label == Tagger::kPosNNPS) &&
        word[word.size() - 1] == 's') {
      return word.substr(0, word.size() - 1);
    }
    return word;
  };

  vector<string> keywords;
  vector<Tagger::Tag> tags = tagger_.Tags(query);
  for (auto beg = tags.cbegin(), end = tags.cend(), it = beg; it != end; ++it) {
    const int label = it->label;
    if (IsNN(label)) {
      if (keywords.empty() ||
          (it - beg > 1 && (it - 1)->label == Tagger::kPosCC &&
           IsNN((it - 2)->label))) {
        // Simple NN or NN conjunction.
        keywords.push_back(
            Singular(query.substr(it->offset.begin, it->offset.size), label));
      } else if (it != beg && IsNN((it - 1)->label)) {
        // Multi-word NN.
        keywords.back() += " " +
          Singular(query.substr(it->offset.begin, it->offset.size), label);
      } else if (keywords.size()) {
        // Found all relevant target keywords.
        break;
      }
    } else if (label == Tagger::kPosPOS && keywords.size()) {
      // Possesive ending, target NN is following.
      keywords.pop_back();
    }
  }
  return keywords;
}

}  // namespace nlp
}  // namespace pyt
