// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include "../nlp/tagger.h"

using std::vector;
using std::set;
using std::string;
using std::cout;
using std::endl;
using std::ofstream;

using ::testing::ElementsAre;
using ::testing::Contains;
using ::testing::Not;

using namespace pyt::nlp;

class TaggerTest : public ::testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }
};

TEST_F(TaggerTest, POS) {
  Tagger pos_tagger(Tagger::kPos);
  {
    vector<Tagger::Tag> tags = pos_tagger.Tags(
        "first targets of the atomic bomb");
    vector<Tagger::Tag> exp =
        {{{0, 5}, Tagger::kPos, Tagger::kPosJJ},
         {{6, 13}, Tagger::kPos, Tagger::kPosNNS},
         {{14, 16}, Tagger::kPos, Tagger::kPosIN},
         {{17, 20}, Tagger::kPos, Tagger::kPosDT},
         {{21, 27}, Tagger::kPos, Tagger::kPosJJ},
         {{28, 32}, Tagger::kPos, Tagger::kPosNN}};
    EXPECT_EQ(exp, tags);
  }
}
