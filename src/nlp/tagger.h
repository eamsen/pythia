// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_TAGGER_H_
#define SRC_NLP_TAGGER_H_

extern "C" {
#include "../../deps/senna/SENNA_Hash.h"
#include "../../deps/senna/SENNA_Tokenizer.h"
#include "../../deps/senna/SENNA_POS.h"
#include "../../deps/senna/SENNA_CHK.h"
#include "../../deps/senna/SENNA_NER.h"
#include "../../deps/senna/SENNA_VBS.h"
#include "../../deps/senna/SENNA_PT0.h"
#include "../../deps/senna/SENNA_SRL.h"
#include "../../deps/senna/SENNA_PSG.h"
}

#include <string>
#include <vector>

namespace pyt {
namespace nlp {

struct Offset {
  size_t begin;
  size_t end;
  bool operator==(const Offset& rhs) const {
    return begin == rhs.begin && end == rhs.end;
  }
};

class Tagger {
 public:
  enum Type {
    kPos = 1u,
    kChk = 2u,
    kNer = 4u,
    kSrl = 8u,
    kPsg = 16u,
    kAll = 32u
  };

  struct Tag {
    Tag(const Offset& offset, Type type, int label)
        : offset(offset), type(type), label(label) {}

    bool operator==(const Tag& rhs) const {
      return offset == rhs.offset && type == rhs.type && label == rhs.label;
    }

    Offset offset;
    Type type;
    int label;
  };

  static size_t kMaxTextSize;
  static size_t kMaxTargetVbSize;
  static std::string SennaPath;

  explicit Tagger(Type type);
  ~Tagger();
  std::vector<Tag> Tags(const std::string& text) const;
  const char* Label(Type type, const int id) const;

 private:
  Type type_;

  // Senna stuff.
  SENNA_Hash* word_hash_;
  SENNA_Hash* caps_hash_;
  SENNA_Hash* suff_hash_;
  SENNA_Hash* gazt_hash_;
  SENNA_Hash* gazl_hash_;
  SENNA_Hash* gazm_hash_;
  SENNA_Hash* gazo_hash_;
  SENNA_Hash* gazp_hash_;
  SENNA_Hash* pos_hash_;
  SENNA_Hash* chk_hash_;
  SENNA_Hash* pt0_hash_;
  SENNA_Hash* ner_hash_;
  SENNA_Hash* vbs_hash_;
  SENNA_Hash* srl_hash_;
  SENNA_Hash* psg_left_hash_;
  SENNA_Hash* psg_right_hash_;
  SENNA_POS* pos_;
  SENNA_CHK* chk_;
  SENNA_PT0* pt0_;
  SENNA_NER* ner_;
  SENNA_VBS* vbs_;
  SENNA_SRL* srl_;
  SENNA_PSG* psg_;
  SENNA_Tokenizer *tokenizer_;
};

}  // namespace nlp
}  // namespace pyt
#endif  // SRC_NLP_TAGGER_H_
