// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_ENTITY_INDEX_H_
#define SRC_NLP_ENTITY_INDEX_H_

#include <unordered_map>
#include <string>
#include <vector>

namespace pyt {
namespace nlp {

struct Entity {
  enum Type {
    kPersonType,
    kLocationType,
    kOrganizationType,
    kMiscType,
    kInvalidType
  };

  bool operator==(const Entity& rhs) const {
    return name == rhs.name && type == rhs.type;
  }

  std::string name;
  Type type;
};

struct EntityHash {
  size_t operator()(const Entity& entity) const {
    return std::hash<std::string>()(entity.name) ^
           std::hash<int>()(entity.type);
  }
};

class EntityIndex {
 public:

  static const int kInvalidId = -1;

  struct Item {
    Item() : score(0.0f) {}

    Item(const float score) : score(score) {}

    bool operator==(const Item& rhs) const {
      return score == rhs.score;
    }

    float score;
  };

  void Add(const std::string& entity, Entity::Type type);
  const std::vector<Item>& Items(const Entity& entity) const;
  std::string Str() const;

 private:
  std::unordered_map<Entity, std::vector<Item>, EntityHash> index_;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_ENTITY_INDEX_H_
