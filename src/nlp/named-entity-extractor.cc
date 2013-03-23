// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./named-entity-extractor.h"
#include <glog/logging.h>
#include <algorithm>
#include <bitset>
#include "./entity-index.h"

using std::string;
using std::vector;
using std::bitset;

namespace pyt {
namespace nlp {

NamedEntityExtractor::NamedEntityExtractor()
    : Tagger(Tagger::kNer) {}

vector<Tagger::Tag> NamedEntityExtractor::Extract(const string& text) const {
  return Extract(text, static_cast<EntityIndex*>(0));
}

vector<Tagger::Tag> NamedEntityExtractor::Extract(
    const string& text, EntityIndex* index) const {
  // kNerO, kNerSLOC, kNerEPER, kNerBPER, kNerSORG, kNerEORG, kNerBORG,
  // kNerSPER,
  // kNerSMISC, kNerIORG, kNerELOC, kNerBLOC, kNerEMISC, kNerBMISC, kNerIPER,
  // kNerIMISC, kNerILOC, kNerPADDING, kNerUNAVAILABLE

  static const vector<Entity::Type> _entity_types =
      {Entity::kInvalidType, Entity::kLocationType, Entity::kPersonType,
       Entity::kPersonType, Entity::kOrganizationType,
       Entity::kOrganizationType, Entity::kOrganizationType,
       Entity::kPersonType, Entity::kMiscType, Entity::kOrganizationType,
       Entity::kLocationType, Entity::kLocationType, Entity::kMiscType,
       Entity::kMiscType, Entity::kPersonType, Entity::kMiscType,
       Entity::kLocationType, Entity::kInvalidType, Entity::kInvalidType};

  static const bitset<kNerUNAVAILABLE + 1> _is_single_word(
      string("0000000000110010010"));

  static const bitset<kNerUNAVAILABLE + 1> _is_entity_begin(
      string("0000010100001001000"));

  static const bitset<kNerUNAVAILABLE + 1> _is_entity_end(
      string("0000001010000100100"));

  vector<Tag> tags = Tags(text);
  vector<Tag> entities;
  for (const Tag& tag: tags) {
    if (tag.label != Tagger::kNerO) {
      entities.push_back(tag);
      if (!index) {
        continue;
      }
      if (_is_single_word[tag.label]) {
        // Single-word entity.
        index->Add(text.substr(tag.offset.begin, tag.offset.size),
                   _entity_types[tag.label], -1, 1.0f);
      } else if (_is_entity_end[tag.label]) {
        // Multi-word entity.
        const int end_index = entities.size();
        int i = end_index - 1;
        while (i > 0 && !_is_entity_begin[entities[i].label]) {
          --i;
        }
        LOG_IF(FATAL, i < 0) << "Entity begin not found.";
        string entity = text.substr(entities[i].offset.begin,
                                    entities[i].offset.size);
        while (++i < end_index) {
          entity += " " + text.substr(entities[i].offset.begin,
                                      entities[i].offset.size);
        }
        index->Add(entity, _entity_types[tag.label], -1, 1.0f);
      }
    }
  }
  return entities;
}

vector<Tagger::Tag> NamedEntityExtractor::Extract(
    const string& text, vector<std::pair<string, Entity::Type> >* index) const {
  // kNerO, kNerSLOC, kNerEPER, kNerBPER, kNerSORG, kNerEORG, kNerBORG,
  // kNerSPER,
  // kNerSMISC, kNerIORG, kNerELOC, kNerBLOC, kNerEMISC, kNerBMISC, kNerIPER,
  // kNerIMISC, kNerILOC, kNerPADDING, kNerUNAVAILABLE

  static const vector<Entity::Type> _entity_types =
      {Entity::kInvalidType, Entity::kLocationType, Entity::kPersonType,
       Entity::kPersonType, Entity::kOrganizationType,
       Entity::kOrganizationType, Entity::kOrganizationType,
       Entity::kPersonType, Entity::kMiscType, Entity::kOrganizationType,
       Entity::kLocationType, Entity::kLocationType, Entity::kMiscType,
       Entity::kMiscType, Entity::kPersonType, Entity::kMiscType,
       Entity::kLocationType, Entity::kInvalidType, Entity::kInvalidType};

  static const bitset<kNerUNAVAILABLE + 1> _is_single_word(
      string("0000000000110010010"));

  static const bitset<kNerUNAVAILABLE + 1> _is_entity_begin(
      string("0000010100001001000"));

  static const bitset<kNerUNAVAILABLE + 1> _is_entity_end(
      string("0000001010000100100"));

  vector<Tag> tags = Tags(text);
  vector<Tag> entities;
  for (const Tag& tag: tags) {
    if (tag.label != Tagger::kNerO) {
      entities.push_back(tag);
      if (!index) {
        continue;
      }
      if (_is_single_word[tag.label]) {
        // Single-word entity.
        index->push_back({text.substr(tag.offset.begin, tag.offset.size),
                          _entity_types[tag.label]}); 
      } else if (_is_entity_end[tag.label]) {
        // Multi-word entity.
        const int end_index = entities.size();
        int i = end_index - 1;
        while (i > 0 && !_is_entity_begin[entities[i].label]) {
          --i;
        }
        LOG_IF(FATAL, i < 0) << "Entity begin not found.";
        string entity = text.substr(entities[i].offset.begin,
                                    entities[i].offset.size);
        while (++i < end_index) {
          entity += " " + text.substr(entities[i].offset.begin,
                                      entities[i].offset.size);
        }
        index->push_back({entity, _entity_types[tag.label]});
      }
    }
  }
  return entities;
}
}  // namespace nlp
}  // namespace pyt
