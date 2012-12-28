// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./named-entity-extractor.h"

using std::string;
using std::vector;

namespace pyt {
namespace nlp {

NamedEntityExtractor::NamedEntityExtractor()
    : Tagger(Tagger::kNer) {}

vector<Tagger::Tag> NamedEntityExtractor::Extract(const string& text) const {
  vector<Tag> tags = Tags(text);
  vector<Tag> entities;
  for (const Tag& tag: tags) {
    if (tag.label != Tagger::kNerO) {
      entities.push_back(tag);
    }
  }
  return entities;
}

}  // namespace nlp
}  // namespace pyt
