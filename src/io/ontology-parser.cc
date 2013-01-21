// Copyright 2013 Eugen Sawin <esawin@me73.com>
#include "./ontology-parser.h"
#include <fstream>
#include <glog/logging.h>

using std::string;

namespace pyt {
namespace io {

int OntologyParser::Parse(const std::string& path) {
  std::ifstream file(path);
  const string relation = ":r:is-a\t:e:entity:Entity\t:e:class:Class";
  string line; 
  while (std::getline(file, line)) {
    if (line.substr(0, relation.size()) == relation) {
      size_t pos = relation.size();
      for (int i = 0; i < 3; ++i) {
        pos = line.find(":", pos + 1);
        LOG_IF(FATAL, pos == string::npos) << "Ontology file is malformed.";
      }
      const string entity_name = line.substr(relation.size() + 4,
                                             pos - relation.size() - 4);
      const size_t begin = line.find("\t", pos) + 4;
      pos = begin - 4;
      for (int i = 0; i < 3; ++i) {
        pos = line.find(":", pos + 1);
        LOG_IF(FATAL, pos == string::npos) << "Ontology file is malformed.";
      }
      const string class_name = line.substr(begin, pos - begin);
    }
  }
  return 0;
}

}  // namespace io
}  // namespace pyt
