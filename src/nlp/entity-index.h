// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_ENTITY_INDEX_H_
#define SRC_NLP_ENTITY_INDEX_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <queue>

namespace pyt {
namespace nlp {

struct Entity {
  enum Type {
    kPersonType = 0,
    kLocationType,
    kOrganizationType,
    kMiscType,
    kInvalidType
  };

  static const std::string& TypeName(const Type type);

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

  class QueueComp {
   public:
    explicit QueueComp(const EntityIndex& index)
        : index_(index) {}

    bool operator()(const Entity& lhs, const Entity& rhs) const {
      auto StrLess = [](const std::string& l, const std::string& r) {
        if (l.size() != r.size()) {
          return l.size() < r.size();
        }
        for (size_t size = l.size(), i = 0; i < size; ++i) {
          if (l[i] != r[i]) {
            return l[i] < r[i];
          }
        }
        return false;
      };
      const size_t lhs_freq = index_.Frequency(lhs);
      const size_t rhs_freq = index_.Frequency(rhs);
      return lhs_freq < rhs_freq ||
          (lhs_freq == rhs_freq && (lhs.type < rhs.type ||
                                    StrLess(lhs.name, rhs.name)));
    }

   private:
    const EntityIndex& index_;
  };

  struct Item {
    Item() : score(0.0f) {}

    Item(const float score) : score(score) {}  // NOLINT

    bool operator==(const Item& rhs) const {
      return score == rhs.score;
    }

    float score;
  };

  EntityIndex();
  void Add(const std::string& entity, Entity::Type type);
  Entity PopTop();
  size_t QueueSize() const;
  size_t Frequency(const Entity& entity) const;
  const std::vector<Item>& Items(const Entity& entity) const;
  std::string Str() const;

 private:
  std::unordered_map<Entity, std::vector<Item>, EntityHash> index_;
  std::priority_queue<Entity, std::vector<Entity>, QueueComp> queue_;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_ENTITY_INDEX_H_
