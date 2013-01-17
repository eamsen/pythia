// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_IO_ONTOLOGY_PARSER_H_
#define SRC_IO_ONTOLOGY_PARSER_H_

#include <string>

namespace pyt {
namespace io {

class OntologyParser {
 public:
  static int Parse(const std::string& path);
};

}  // namespace io
}  // namespace pyt
#endif  // SRC_IO_ONTOLOGY_PARSER_H_
