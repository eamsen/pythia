// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NET_SERVER_H_
#define SRC_NET_SERVER_H_

#include <Poco/Util/ServerApplication.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "../nlp/tagger.h"
#include "../nlp/ontology-index.h"

namespace pyt {
namespace net {

class Server: public Poco::Util::ServerApplication {
 public:
  Server(const std::string& name, const std::string& version,
         const std::string& doc_path, const uint16_t port,
         const uint16_t threads, const uint16_t queue_size);
  ~Server();
  void Run();
  const std::string& DocumentPath() const;
  const std::string& ApiKey() const;
  const std::string& ApiCx() const;
  const std::string& SearchHost() const;
  const std::string& SearchBase() const;
  const pyt::nlp::Tagger& Tagger() const;
  const pyt::nlp::OntologyIndex& OntologyIndex() const;
  std::unordered_map<std::string, std::string>& WebCache();

 private:
  void initialize(Poco::Util::Application& self);  // NOLINT
  void uninitialize();
  int main(const std::vector<std::string>& args);

  std::string name_;
  std::string version_;
  std::string doc_path_;
  uint32_t port_;
  uint32_t num_threads_;
  uint16_t queue_size_;
  std::string api_key_;
  std::string api_cx_;
  std::string search_host_;
  std::string search_base_;
  pyt::nlp::Tagger tagger_;
  pyt::nlp::OntologyIndex ontology_index_;
  std::unordered_map<std::string, std::string> web_cache_;
};

}  // namespace net
}  // namespace pyt
#endif  // SRC_NET_SERVER_H_
