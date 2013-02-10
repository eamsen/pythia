// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_NAMED_ENTITY_EXTRACTOR_H_
#define SRC_NLP_NAMED_ENTITY_EXTRACTOR_H_

#include <string>
#include <vector>
#include "./tagger.h"
#include "./entity-index.h"

namespace pyt {
namespace nlp {

class EntityIndex;

class NamedEntityExtractor: public Tagger {
 public:
  NamedEntityExtractor();
  std::vector<Tag> Extract(const std::string& text) const;
  std::vector<Tag> Extract(const std::string& text, EntityIndex* index) const;
  std::vector<Tag> Extract(const std::string& text,
      std::vector<std::pair<std::string, Entity::Type> >* index) const;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_NAMED_ENTITY_EXTRACTOR_H_
