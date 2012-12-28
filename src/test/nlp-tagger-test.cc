// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include "../nlp/tagger.h"
#include "../nlp/named-entity-extractor.h"

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

TEST_F(TaggerTest, Pos) {
  Tagger tagger(Tagger::kPos);
  {
    vector<Tagger::Tag> tags = tagger.Tags("");
    vector<Tagger::Tag> exp;
    EXPECT_EQ(exp, tags);
  }
  {
    vector<Tagger::Tag> tags = tagger.Tags(
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

TEST_F(TaggerTest, Ner) {
  Tagger tagger(Tagger::kNer);
  NamedEntityExtractor extractor;
  {
    vector<Tagger::Tag> exp;
    EXPECT_EQ(exp, tagger.Tags(""));
    EXPECT_EQ(exp, extractor.Extract(""));
  }
  {
    string text = "Internet giant Google has made a very high-profile hire, ";
    text += "bringing famed inventor and futurist Ray Kurzweil on board as ";
    text += "the company's new director of engineering.";
    vector<Tagger::Tag> tags = tagger.Tags(text);
    vector<Tagger::Tag> exp =
        {{{0, 8}, Tagger::kNer, Tagger::kNerO},
         {{9, 14}, Tagger::kNer, Tagger::kNerO},
         {{15, 21}, Tagger::kNer, Tagger::kNerSORG},
         {{22, 25}, Tagger::kNer, Tagger::kNerO},
         {{26, 30}, Tagger::kNer, Tagger::kNerO},
         {{31, 32}, Tagger::kNer, Tagger::kNerO},
         {{33, 37}, Tagger::kNer, Tagger::kNerO},
         {{38, 50}, Tagger::kNer, Tagger::kNerO},
         {{51, 55}, Tagger::kNer, Tagger::kNerO},
         {{55, 56}, Tagger::kNer, Tagger::kNerO},
         {{57, 65}, Tagger::kNer, Tagger::kNerO},
         {{66, 71}, Tagger::kNer, Tagger::kNerO},
         {{72, 80}, Tagger::kNer, Tagger::kNerO},
         {{81, 84}, Tagger::kNer, Tagger::kNerO},
         {{85, 93}, Tagger::kNer, Tagger::kNerO},
         {{94, 97}, Tagger::kNer, Tagger::kNerBPER},
         {{98, 106}, Tagger::kNer, Tagger::kNerEPER},
         {{107, 109}, Tagger::kNer, Tagger::kNerO},
         {{110, 115}, Tagger::kNer, Tagger::kNerO},
         {{116, 118}, Tagger::kNer, Tagger::kNerO},
         {{119, 122}, Tagger::kNer, Tagger::kNerO},
         {{123, 130}, Tagger::kNer, Tagger::kNerO},
         {{130, 132}, Tagger::kNer, Tagger::kNerO},
         {{133, 136}, Tagger::kNer, Tagger::kNerO},
         {{137, 145}, Tagger::kNer, Tagger::kNerO},
         {{146, 148}, Tagger::kNer, Tagger::kNerO},
         {{149, 160}, Tagger::kNer, Tagger::kNerO},
         {{160, 161}, Tagger::kNer, Tagger::kNerO}};
    EXPECT_EQ(exp, tags);
    vector<Tagger::Tag> entities = extractor.Extract(text);
    exp = {{{15, 21}, Tagger::kNer, Tagger::kNerSORG},
           {{94, 97}, Tagger::kNer, Tagger::kNerBPER},
           {{98, 106}, Tagger::kNer, Tagger::kNerEPER}};
    EXPECT_EQ(exp, entities);
  }
}
