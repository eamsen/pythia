// Copyright 2013 Eugen Sawin <esawin@me73.com>
#include "./ontology-index.h"
#include <glog/logging.h>
#include <sstream>
#include "../io/serialize.h"

using std::vector;
using std::unordered_set;
using std::pair;
using std::string;

namespace pyt {
namespace nlp {

int OntologyIndex::ParseFromCsv(std::istream& stream,  // NOLINT
                                const unordered_set<string>& filter,
                                OntologyIndex* index) {
  static const string relation = ":r:is-a\t:e:entity:Entity\t:e:class:Class";
  LOG_IF(FATAL, !index) << "Null pointer passed as index.";

  auto Filter = [&filter](const string& name) {
    if (name.size() < 3 || filter.count(name)) {
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

int OntologyIndex::ParseScoresFromCsv(std::istream& stream,  // NOLINT
                                      OntologyIndex* index) {
  LOG_IF(FATAL, !index) << "Null pointer passed as index.";
  string line;
  while (std::getline(stream, line)) {
    if (line.substr(0, 3) == ":e:") {
      size_t pos = line.find(":", 3);
      LOG_IF(FATAL, pos == string::npos) << "Scores stream is malformed.";
      const string entity_name = line.substr(3, pos - 3);
      const size_t begin = line.find("\t", pos);
      LOG_IF(FATAL, begin == string::npos) << "Scores stream is malformed.";
      pos = line.find("\t", begin + 1);
      LOG_IF(FATAL, pos == string::npos) << "Scores stream is malformed.";
      int score = 0;
      std::stringstream(line.substr(begin + 1, pos - begin - 1)) >> score;
      const int entity_id = index->LhsNameId(entity_name);
      if (entity_id != OntologyIndex::kInvalidId) {
        index->LhsFrequency(entity_id, score);
      }
    }
  }
  return index->SumLhsFrequencies();
}

OntologyIndex::OntologyIndex()
    : num_lhs_triples_(0),
      sum_lhs_freq_(0) {}

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
  ++rhs_freqs_[rhs_id];
  rhs_ids_[rhs] = rhs_id;
  DLOG_IF(FATAL, lhs_triples_.size() != names_.size())
      << "Error in LHS triple indexing.";
  lhs_triples_[lhs_id].push_back({rel_id, rhs_id});
  ++num_lhs_triples_;
}

int OntologyIndex::AddRelation(const string& name) {
  DLOG_IF(WARNING, relation_ids_.count(name)) << "Redundant relation adding.";
  relation_ids_[name] = relations_.size();
  relations_.push_back(name);
  return relations_.size() - 1;
}

int OntologyIndex::AddName(const string& name) {
  names_.push_back(name);
  rhs_freqs_.push_back(0);
  lhs_freqs_.push_back(0);
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

const string& OntologyIndex::Name(const int id) const {
  DLOG_IF(FATAL, id < 0 || id >= static_cast<int>(names_.size()));
  return names_[id];
}

const vector<pair<int32_t, int32_t> >& OntologyIndex::RelationsByLhs(
    const string& lhs) const {
  static const vector<pair<int32_t, int32_t> > empty;
  const int lhs_id = LhsNameId(lhs);
  if (lhs_id == kInvalidId) {
    return empty;
  }
  return RelationsByLhs(lhs_id);
}

const vector<pair<int32_t, int32_t> >& OntologyIndex::RelationsByLhs(
    const int lhs_id) const {
  DLOG_IF(FATAL, lhs_id < 0 || lhs_id >= static_cast<int>(lhs_triples_.size()))
      << "Index out of bounds";
  return lhs_triples_[lhs_id];
}
  
int OntologyIndex::NumTriples() const {
  return num_lhs_triples_;
}

int OntologyIndex::SumLhsFrequencies() const {
  return sum_lhs_freq_;
}

void OntologyIndex::LhsFrequency(const int lhs_id, const int freq) {
  DLOG_IF(FATAL, lhs_id < 0 || lhs_id >= static_cast<int>(lhs_freqs_.size()))
      << "Index out of bounds";
  sum_lhs_freq_ += freq - lhs_freqs_[lhs_id];
  lhs_freqs_[lhs_id] = freq;
}

int OntologyIndex::LhsFrequency(const int lhs_id) const {
  if (lhs_id < 0 || lhs_id >= static_cast<int>(lhs_freqs_.size())) {
    return 0;
  }
  return lhs_freqs_[lhs_id];
}

int OntologyIndex::RhsFrequency(const int rhs_id) const {
  DLOG_IF(FATAL, rhs_id < 0 || rhs_id >= static_cast<int>(rhs_freqs_.size()))
      << "Index out of bounds";
  return rhs_freqs_[rhs_id];
}

void OntologyIndex::Save(std::ostream& stream) const {  // NOLINT
  using pyt::io::Write;
  Write(lhs_ids_, stream);
  Write(rhs_ids_, stream);
  Write(relation_ids_, stream);
  Write(names_, stream);
  Write(relations_, stream);
  Write(lhs_triples_, stream);
  Write(num_lhs_triples_, stream);
  Write(sum_lhs_freq_, stream);
  Write(lhs_freqs_, stream);
  Write(rhs_freqs_, stream);
}

void OntologyIndex::Load(std::istream& stream) {  // NOLINT
  using pyt::io::Read;
  Read(stream, &lhs_ids_);
  Read(stream, &rhs_ids_);
  Read(stream, &relation_ids_);
  Read(stream, &names_);
  Read(stream, &relations_);
  Read(stream, &lhs_triples_);
  Read(stream, &num_lhs_triples_);
  Read(stream, &sum_lhs_freq_);
  Read(stream, &lhs_freqs_);
  Read(stream, &rhs_freqs_);
}

}  // namespace nlp
}  // namespace pyt
