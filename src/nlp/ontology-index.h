// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_ONTOLOGY_INDEX_H_
#define SRC_NLP_ONTOLOGY_INDEX_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <istream>
#include <ostream>

namespace pyt {
namespace nlp {

class OntologyIndex {
 public:
  static const int kInvalidId = -1;

  static int ParseFromCsv(std::istream& stream,  // NOLINT
                          const std::unordered_set<std::string>& filter,
                          OntologyIndex* index);

  OntologyIndex();
  void AddTriple(const std::string& relation, const std::string& lhs,
                 const std::string& rhs);
  int AddRelation(const std::string& name);
  int AddName(const std::string& name);
  int LhsNameId(const std::string& name) const;
  int RhsNameId(const std::string& name) const;
  int NameId(const std::string& name) const;
  int RelationId(const std::string& name) const;
  int NumTriples() const;
  int RhsFreq(const int rhs_id) const;
  const std::string& Name(const int id) const;
  const std::vector<std::pair<int32_t, int32_t> >& RelationsByLhs(
      const std::string& lhs) const;
  const std::vector<std::pair<int32_t, int32_t> >& RelationsByLhs(
      const int lhs_id) const;
  void Save(std::ostream& stream) const;  // NOLINT
  void Load(std::istream& stream);  // NOLINT

 private:
  std::unordered_map<std::string, int32_t> lhs_ids_;
  std::unordered_map<std::string, int32_t> rhs_ids_;
  std::unordered_map<std::string, int32_t> relation_ids_;
  std::vector<std::string> names_;
  std::vector<std::string> relations_;
  std::vector<std::vector<std::pair<int32_t, int32_t> > > lhs_triples_;
  int32_t num_lhs_triples_;
  std::vector<int32_t> rhs_freqs_;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_ONTOLOGY_INDEX_H_
