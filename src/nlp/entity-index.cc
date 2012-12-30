// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./entity-index.h"
#include <glog/logging.h>
#include <algorithm>
#include <sstream>

using std::string;
using std::unordered_map;
using std::vector;

namespace pyt {
namespace nlp {

EntityIndex::EntityIndex()
    : queue_(QueueComp(*this)) {}

void EntityIndex::Add(const string& _entity, Entity::Type type) {
  Entity entity({_entity, type});
  std::transform(entity.name.begin(), entity.name.end(), entity.name.begin(),
                 ::tolower);
  auto it = index_.find(entity);
  if (it == index_.end()) {
    index_[entity].push_back({1.0f});
  } else {
    it->second.push_back({it->second.back().score + 1.0f});
  }
  queue_.push(entity);
}

Entity EntityIndex::PopTop() {
  LOG_IF(FATAL, queue_.empty()) << "Empty entity index queue.";
  Entity top = queue_.top();
  queue_.pop();
  return top;
}

size_t EntityIndex::QueueSize() const {
  return queue_.size();
}

size_t EntityIndex::Frequency(const Entity& entity) const {
  auto it = index_.find(entity);
  if (it == index_.end()) {
    return 0;
  }
  return it->second.size();
}

auto EntityIndex::Items(const Entity& entity) const -> const vector<Item>& {
  static const vector<Item> _empty;
  auto it = index_.find(entity);
  if (it == index_.cend()) {
    return _empty;
  }
  return it->second;
}

string EntityIndex::Str() const {
  std::ostringstream ss;
  for (auto beg = index_.cbegin(), end = index_.cend(), it = beg;
       it != end; ++it) {
    if (it != beg) {
      ss << ", ";
    }
    ss << it->first.name << " (" << it->first.type << "): "
       << it->second.back().score;
  }
  return ss.str();
}

}  // namespace nlp
}  // namespace pyt
