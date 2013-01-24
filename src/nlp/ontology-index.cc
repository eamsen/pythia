// Copyright 2013 Eugen Sawin <esawin@me73.com>
#include "./ontology-index.h"
#include <glog/logging.h>

using std::string;
using std::vector;
using std::unordered_map;

namespace pyt {
namespace nlp {

int OntologyIndex::ParseFromCsv(std::istream& stream, OntologyIndex* index) {
  static const string relation = ":r:is-a\t:e:entity:Entity\t:e:class:Class";
  LOG_IF(FATAL, !index) << "Null pointer passed as index.";

  auto Filter = [](const string& name) {
    if (name.size() < 3) {
      return false;
    }
    for (const char c: name) {
      if (!std::isalpha(c)) {
        return false;
      }
    }
    return true;
  };

  int num_triples = 0;
  string line; 
  while (std::getline(stream, line)) {
    if (line.substr(0, relation.size()) == relation) {
      size_t pos = relation.size();
      for (int i = 0; i < 3; ++i) {
        pos = line.find(":", pos + 1);
        LOG_IF(FATAL, pos == string::npos) << "Ontology stream is malformed.";
      }
      const string entity_name = line.substr(relation.size() + 4,
                                             pos - relation.size() - 4);
      const size_t begin = line.find("\t", pos) + 4;
      pos = begin - 4;
      for (int i = 0; i < 3; ++i) {
        pos = line.find(":", pos + 1);
        LOG_IF(FATAL, pos == string::npos) << "Ontology stream is malformed.";
      }
      const string class_name = line.substr(begin, pos - begin);
      if (Filter(entity_name) && Filter(class_name)) {
        index->AddTriple("is-a", entity_name, class_name);
        ++num_triples;
      }
    }
  }
  return num_triples;
}

void OntologyIndex::AddTriple(const std::string& relation,
    const std::string& lhs, const std::string& rhs) {
  int rel_id = RelationId(relation);
  if (rel_id == kInvalidId) {
    rel_id = AddRelation(relation);
  }
  int lhs_id = NameId(lhs);  
  if (lhs_id == kInvalidId) {
    lhs_id = AddName(lhs);
    lhs_triples_.push_back({});
  }
  lhs_ids_[lhs] = lhs_id;
  int rhs_id = NameId(rhs);  
  if (rhs_id == kInvalidId) {
    rhs_id = AddName(rhs);
    // TODO(esawin): Make that more memory-efficient.
    lhs_triples_.push_back({});
  }
  rhs_ids_[rhs] = rhs_id;
  LOG_IF(FATAL, lhs_triples_.size() != names_.size())
    << "Error in LHS triple indexing.";
  lhs_triples_[lhs_id].push_back({rel_id, rhs_id});
}

int OntologyIndex::AddRelation(const string& name) {
  LOG_IF(WARNING, relation_ids_.count(name)) << "Redundant relation adding.";
  relation_ids_[name] = relations_.size();
  relations_.push_back(name);
  return relations_.size() - 1;
}

int OntologyIndex::AddName(const string& name) {
  names_.push_back(name);
  return names_.size() - 1;
}

int OntologyIndex::LhsNameId(const string& name) const {
  const auto it = lhs_ids_.find(name);
  if (it == lhs_ids_.end()) {
    return kInvalidId;
  }
  return it->second;
}

int OntologyIndex::RhsNameId(const string& name) const {
  const auto it = rhs_ids_.find(name);
  if (it == rhs_ids_.end()) {
    return kInvalidId;
  }
  return it->second;
}

int OntologyIndex::NameId(const string& name) const {
  int id = LhsNameId(name);
  if (id == kInvalidId) {
    id = RhsNameId(name);
  }
  return id;
}

int OntologyIndex::RelationId(const string& name) const {
  const auto it = relation_ids_.find(name);
  if (it == relation_ids_.end()) {
    return kInvalidId;
  }
  return it->second;
}

}  // namespace nlp
}  // namespace pyt
