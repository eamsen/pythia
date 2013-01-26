// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NLP_TAGGER_H_
#define SRC_NLP_TAGGER_H_

#include <string>
#include <vector>

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

namespace pyt {
namespace nlp {

struct Offset {
  size_t begin;
  size_t size;
  bool operator==(const Offset& rhs) const {
    return begin == rhs.begin && size == rhs.size;
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
    kAll = 31u
  };

  enum PosLabel {
    kPosNNP = 0,
    kPosComma, kPosCD, kPosNNS, kPosJJ, kPosMD, kPosVB, kPosDT, kPosNN, kPosIN,
    kPosFullStop, kPosVBZ, kPosVBG, kPosCC, kPosVBD, kPosVBN, kPosRB, kPosTO,
    kPosPRP, kPosRBR, kPosWDT, kPosVBP, kPosRP, kPosPRPE, kPosJJS, kPosPOS,
    kPosQuote1, kPosWP, kPosQuote2, kPosColon, kPosJJR, kPosWRB, kPosEX, kPosE,
    kPosNNPS, kPosWPE, kPosLRB, kPosRRB, kPosPDT, kPosRBS, kPosFW, kPosUH,
    kPosSYM, kPosLS, kPosHash, kPosPADDING, kPosUNAVAILABLE
  };

  enum ChkLabel {
    kChkENP = 0,
    kChkBNP, kChkO, kChkINP, kChkSPP, kChkSNP, kChkSVP, kChkEVP, kChkBVP,
    kChkIVP, kChkSADVP, kChkSBAR, kChkSADJP, kChkSPRT, kChkEADJP, kChkBADJP,
    kChkEADVP, kChkBADVP, kChkEPP, kChkBPP, kChkIADJP, kChkESBAR, kChkBSBAR,
    kChkIADVP, kChkECONJP, kChkCONJP, kChkSINTJ, kChkICONJP, kChkIPP, kChkSLST,
    kChkEINTJ, kChkBINTJ, kChkSCONJP, kChkIUCP, kChkIINTJ, kChkEUCP, kChkBUCP,
    kChkIPRT, kChkEPRT, kChkBPRT, kChkBLST, kChkELST, kChkPADDING,
    kChkUNAVAILABLE
  };

  enum NerLabel {
    kNerO = 0,
    kNerSLOC, kNerEPER, kNerBPER, kNerSORG, kNerEORG, kNerBORG, kNerSPER,
    kNerSMISC, kNerIORG, kNerELOC, kNerBLOC, kNerEMISC, kNerBMISC, kNerIPER,
    kNerIMISC, kNerILOC, kNerPADDING, kNerUNAVAILABLE
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

  explicit Tagger(uint8_t type = kPos);
  ~Tagger();
  Tagger& operator=(const Tagger& rhs);
  std::vector<Tag> Tags(const std::string& text) const;
  const char* Label(Type type, const int id) const;

 private:
  uint8_t type_;

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
