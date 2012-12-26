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

using namespace pyt::nlp;

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
  EXPECT_EQ(vector<string>({"target"}),
            qa.TargetKeywords("first targets of the atomic bomb"));
  EXPECT_EQ(vector<string>({"Apollo astronaut"}),
            qa.TargetKeywords("Apollo astronauts who walked on the Moon"));
  EXPECT_EQ(vector<string>({"state"}),
            qa.TargetKeywords("Arab states of the Persian Gulf"));
  EXPECT_EQ(vector<string>({"astronaut"}),
            qa.TargetKeywords("astronauts who landed on the Moon"));
  EXPECT_EQ(vector<string>({"Axis power"}),
            qa.TargetKeywords("Axis powers of World War II"));
  EXPECT_EQ(vector<string>({"book"}),
            qa.TargetKeywords("books of the Jewish canon"));
  EXPECT_EQ(vector<string>({"borough"}),
            qa.TargetKeywords("boroughs of New York City"));
  EXPECT_EQ(vector<string>({"branch"}),
            qa.TargetKeywords("Branches of the US military"));
  EXPECT_EQ(vector<string>({"continent"}),
            qa.TargetKeywords("continents in the world"));
  EXPECT_EQ(vector<string>({"degree"}),
            qa.TargetKeywords("degrees of Eastern Orthodox monasticism"));
  EXPECT_EQ(vector<string>({"sibling"}),
            qa.TargetKeywords("did nicole kidman have any siblings"));
  EXPECT_EQ(vector<string>({"diocese"}),
            qa.TargetKeywords("dioceses of the church of ireland"));
  EXPECT_EQ(vector<string>({"epic"}),
            qa.TargetKeywords("five great epics of Tamil literature"));
  EXPECT_EQ(vector<string>({"god"}),
            qa.TargetKeywords("gods who dwelt on Mount Olympus"));
  EXPECT_EQ(vector<string>({"brother", "sister"}),
            qa.TargetKeywords("henry ii's brothers and sisters"));
  EXPECT_EQ(vector<string>({"hijacker"}),
            qa.TargetKeywords("hijackers in the September 11 attacks"));
  EXPECT_EQ(vector<string>({"house"}),
            qa.TargetKeywords("houses of the Russian parliament"));
  EXPECT_EQ(vector<string>({"parent"}),
            qa.TargetKeywords("john lennon, parents"));
  EXPECT_EQ(vector<string>({"captain"}),
            qa.TargetKeywords("kenya's captain in cricket"));
  EXPECT_EQ(vector<string>({"sibling"}),
            qa.TargetKeywords("kublai khan siblings"));
  EXPECT_EQ(vector<string>({"parent"}),
            qa.TargetKeywords("lilly allen parents"));
  EXPECT_EQ(vector<string>({"league"}),
            qa.TargetKeywords("major leagues in the united states"));
  EXPECT_EQ(vector<string>({"parent"}),
            qa.TargetKeywords("manfred von richthofen parents"));
  EXPECT_EQ(vector<string>({}),
            qa.TargetKeywords(""));
}
