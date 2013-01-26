// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include "../nlp/query-analyser.h"
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

using namespace pyt::nlp;  // NOLINT

class QueryAnalyserTest : public ::testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }
  Tagger pos_tagger;
};

TEST_F(QueryAnalyserTest, TargetKeywords) {
  QueryAnalyser qa(pos_tagger);
  EXPECT_EQ(vector<string>({"targets"}),
            qa.TargetKeywords("first targets of the atomic bomb"));
  EXPECT_EQ(vector<string>({"Apollo astronauts"}),
            qa.TargetKeywords("Apollo astronauts who walked on the Moon"));
  EXPECT_EQ(vector<string>({"states"}),
            qa.TargetKeywords("Arab states of the Persian Gulf"));
  EXPECT_EQ(vector<string>({"astronauts"}),
            qa.TargetKeywords("astronauts who landed on the Moon"));
  EXPECT_EQ(vector<string>({"Axis powers"}),
            qa.TargetKeywords("Axis powers of World War II"));
  EXPECT_EQ(vector<string>({"books"}),
            qa.TargetKeywords("books of the Jewish canon"));
  EXPECT_EQ(vector<string>({"boroughs"}),
            qa.TargetKeywords("boroughs of New York City"));
  EXPECT_EQ(vector<string>({"Branches"}),
            qa.TargetKeywords("Branches of the US military"));
  EXPECT_EQ(vector<string>({"continents"}),
            qa.TargetKeywords("continents in the world"));
  EXPECT_EQ(vector<string>({"degrees"}),
            qa.TargetKeywords("degrees of Eastern Orthodox monasticism"));
  // EXPECT_EQ(vector<string>({"sibling"}),
  //           qa.TargetKeywords("did nicole kidman have any siblings"));
  EXPECT_EQ(vector<string>({"dioceses"}),
            qa.TargetKeywords("dioceses of the church of ireland"));
  EXPECT_EQ(vector<string>({"epics"}),
            qa.TargetKeywords("five great epics of Tamil literature"));
  EXPECT_EQ(vector<string>({"gods"}),
            qa.TargetKeywords("gods who dwelt on Mount Olympus"));
  EXPECT_EQ(vector<string>({"brothers", "sisters"}),
            qa.TargetKeywords("henry ii's brothers and sisters"));
  EXPECT_EQ(vector<string>({"hijackers"}),
            qa.TargetKeywords("hijackers in the September 11 attacks"));
  EXPECT_EQ(vector<string>({"houses"}),
            qa.TargetKeywords("houses of the Russian parliament"));
  EXPECT_EQ(vector<string>({"parents"}),
            qa.TargetKeywords("john lennon, parents"));
  EXPECT_EQ(vector<string>({"captain"}),
            qa.TargetKeywords("kenya's captain in cricket"));
  EXPECT_EQ(vector<string>({"siblings"}),
            qa.TargetKeywords("kublai khan siblings"));
  EXPECT_EQ(vector<string>({"parents"}),
            qa.TargetKeywords("lilly allen parents"));
  EXPECT_EQ(vector<string>({"leagues"}),
            qa.TargetKeywords("major leagues in the united states"));
  EXPECT_EQ(vector<string>({"parents"}),
            qa.TargetKeywords("manfred von richthofen parents"));
  EXPECT_EQ(vector<string>({}),
            qa.TargetKeywords(""));
}
