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
  Tagger::SennaPath = "deps/senna/";
  Tagger tagger(Tagger::kPos);
  {
    vector<Tagger::Tag> tags = tagger.Tags("first targets of the atomic bomb");
    vector<Tagger::Tag> exp;
    exp.push_back(Tagger::Tag({0, 5}, Tagger::kPos, 0));
    EXPECT_EQ(exp[0], tags[0]);
  }
}
