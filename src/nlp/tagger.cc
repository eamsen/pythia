// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./tagger.h"
#include <glog/logging.h>

extern "C" {
#include "../../deps/senna/SENNA_utils.h"
}

using std::string;
using std::vector;

namespace pyt {
namespace nlp {

size_t Tagger::kMaxTextSize = 1024;
size_t Tagger::kMaxTargetVbSize = 256;
string Tagger::SennaPath = "deps/senna/";  // NOLINT

Tagger::Tagger(uint8_t type)
    : type_(type) {
  SENNA_set_verbose_mode(false);

  word_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/words.lst");
  caps_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/caps.lst");
  suff_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/suffix.lst");
  gazt_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/gazetteer.lst");

  gazl_hash_ = SENNA_Hash_new_with_admissible_keys(SennaPath.c_str(),
      "hash/ner.loc.lst", "data/ner.loc.dat");
  gazm_hash_ = SENNA_Hash_new_with_admissible_keys(SennaPath.c_str(),
      "hash/ner.msc.lst", "data/ner.msc.dat");
  gazo_hash_ = SENNA_Hash_new_with_admissible_keys(SennaPath.c_str(),
      "hash/ner.org.lst", "data/ner.org.dat");
  gazp_hash_ = SENNA_Hash_new_with_admissible_keys(SennaPath.c_str(),
      "hash/ner.per.lst", "data/ner.per.dat");

  pos_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/pos.lst");
  chk_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/chk.lst");
  pt0_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/pt0.lst");
  ner_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/ner.lst");
  vbs_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/vbs.lst");
  srl_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/srl.lst");
  psg_left_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/psg-left.lst");
  psg_right_hash_ = SENNA_Hash_new(SennaPath.c_str(), "hash/psg-right.lst");

  pos_ = SENNA_POS_new(SennaPath.c_str(), "data/pos.dat");
  chk_ = SENNA_CHK_new(SennaPath.c_str(), "data/chk.dat");
  pt0_ = SENNA_PT0_new(SennaPath.c_str(), "data/pt0.dat");
  ner_ = SENNA_NER_new(SennaPath.c_str(), "data/ner.dat");
  vbs_ = SENNA_VBS_new(SennaPath.c_str(), "data/vbs.dat");
  srl_ = SENNA_SRL_new(SennaPath.c_str(), "data/srl.dat");
  psg_ = SENNA_PSG_new(SennaPath.c_str(), "data/psg.dat");

  tokenizer_ = SENNA_Tokenizer_new(word_hash_, caps_hash_, suff_hash_,
      gazt_hash_, gazl_hash_, gazm_hash_, gazo_hash_, gazp_hash_, false);
}

Tagger::~Tagger() {
  SENNA_Tokenizer_free(tokenizer_);

  SENNA_POS_free(pos_);
  SENNA_CHK_free(chk_);
  SENNA_PT0_free(pt0_);
  SENNA_NER_free(ner_);
  SENNA_VBS_free(vbs_);
  SENNA_SRL_free(srl_);
  SENNA_PSG_free(psg_);

  SENNA_Hash_free(word_hash_);
  SENNA_Hash_free(caps_hash_);
  SENNA_Hash_free(suff_hash_);
  SENNA_Hash_free(gazt_hash_);

  SENNA_Hash_free(gazl_hash_);
  SENNA_Hash_free(gazm_hash_);
  SENNA_Hash_free(gazo_hash_);
  SENNA_Hash_free(gazp_hash_);

  SENNA_Hash_free(pos_hash_);
  SENNA_Hash_free(chk_hash_);
  SENNA_Hash_free(pt0_hash_);
  SENNA_Hash_free(ner_hash_);
  SENNA_Hash_free(vbs_hash_);
  SENNA_Hash_free(srl_hash_);
  SENNA_Hash_free(psg_left_hash_);
  SENNA_Hash_free(psg_right_hash_);
}

Tagger& Tagger::operator=(const Tagger& rhs) {
  type_ = rhs.type_;
  return *this;
}

vector<Tagger::Tag> Tagger::Tags(const string& text) const {
  vector<Tag> tags;
  if (text.empty()) {
    return tags;
  }
  // TODO(esawin): Is this thread-safe?
  SENNA_Tokens* tokens = SENNA_Tokenizer_tokenize(tokenizer_, text.c_str());
  if (tokens->n == 0) {
    LOG(WARNING) << "Tokenizer failed.";
    return tags;
  }
  // TODO(esawin): Is this thread-safe?
  if (type_ & kPos) {
    // Part-of-speech tagging.
    int* pos_labels = SENNA_POS_forward(pos_, tokens->word_idx,
        tokens->caps_idx, tokens->suff_idx, tokens->n);
    tags.reserve(tokens->n);
    for (int i = 0; i < tokens->n; ++i) {
      Offset offset = {tokens->start_offset[i],
                       tokens->end_offset[i] - tokens->start_offset[i]};
      tags.push_back({offset, kPos, pos_labels[i]});
    }
  }
  if (type_ & kNer) {
    // Named entity recognition.
    int* ner_labels = SENNA_NER_forward(ner_, tokens->word_idx,
        tokens->caps_idx, tokens->gazl_idx, tokens->gazm_idx, tokens->gazo_idx,
        tokens->gazp_idx, tokens->n);
    for (int i = 0; i < tokens->n; ++i) {
      Offset offset = {tokens->start_offset[i],
                       tokens->end_offset[i] - tokens->start_offset[i]};
      tags.push_back({offset, kNer, ner_labels[i]});
    }
  }
  return tags;
}

const char* Tagger::Label(Tagger::Type type, const int id) const {
  if (type == kPos) {
    return pos_hash_->keys[id];
  } else if (type == kChk) {
    return chk_hash_->keys[id];
  }
  return "";
}

}  // namespace nlp
}  // namespace pyt
