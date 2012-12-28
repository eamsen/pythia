// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_NAMED_ENTITY_EXTRACTOR_H_
#define SRC_NLP_NAMED_ENTITY_EXTRACTOR_H_

#include <string>
#include <vector>
#include "./tagger.h"

namespace pyt {
namespace nlp {

class NamedEntityExtractor: public Tagger {
 public:
  NamedEntityExtractor();
  std::vector<Tag> Extract(const std::string& text) const;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_NAMED_ENTITY_EXTRACTOR_H_
