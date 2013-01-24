// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_ONTOLOGY_INDEX_H_
#define SRC_NLP_ONTOLOGY_INDEX_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <istream>

namespace pyt {
namespace nlp {

class OntologyIndex {
 public:
  static const int kInvalidId = -1;

  static int ParseFromCsv(std::istream& stream, OntologyIndex* index);

  void AddTriple(const std::string& relation,
      const std::string& lhs, const std::string& rhs);
  int AddRelation(const std::string& name);
  int AddName(const std::string& name);
  int LhsNameId(const std::string& name) const;
  int RhsNameId(const std::string& name) const;
  int NameId(const std::string& name) const;
  int RelationId(const std::string& name) const;

 private:
  std::unordered_map<std::string, int> lhs_ids_;
  std::unordered_map<std::string, int> rhs_ids_;
  std::unordered_map<std::string, int> relation_ids_;
  std::vector<std::string> names_;
  std::vector<std::string> relations_;
  std::vector<std::vector<std::pair<int, int> > > lhs_triples_;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_ONTOLOGY_INDEX_H_
