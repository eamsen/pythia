// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include "../nlp/tagger.h"
#include "../nlp/named-entity-extractor.h"
#include "../nlp/entity-index.h"

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
         {{6, 7}, Tagger::kPos, Tagger::kPosNNS},
         {{14, 2}, Tagger::kPos, Tagger::kPosIN},
         {{17, 3}, Tagger::kPos, Tagger::kPosDT},
         {{21, 6}, Tagger::kPos, Tagger::kPosJJ},
         {{28, 4}, Tagger::kPos, Tagger::kPosNN}};
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
         {{9, 5}, Tagger::kNer, Tagger::kNerO},
         {{15, 6}, Tagger::kNer, Tagger::kNerSORG},
         {{22, 3}, Tagger::kNer, Tagger::kNerO},
         {{26, 4}, Tagger::kNer, Tagger::kNerO},
         {{31, 1}, Tagger::kNer, Tagger::kNerO},
         {{33, 4}, Tagger::kNer, Tagger::kNerO},
         {{38, 12}, Tagger::kNer, Tagger::kNerO},
         {{51, 4}, Tagger::kNer, Tagger::kNerO},
         {{55, 1}, Tagger::kNer, Tagger::kNerO},
         {{57, 8}, Tagger::kNer, Tagger::kNerO},
         {{66, 5}, Tagger::kNer, Tagger::kNerO},
         {{72, 8}, Tagger::kNer, Tagger::kNerO},
         {{81, 3}, Tagger::kNer, Tagger::kNerO},
         {{85, 8}, Tagger::kNer, Tagger::kNerO},
         {{94, 3}, Tagger::kNer, Tagger::kNerBPER},
         {{98, 8}, Tagger::kNer, Tagger::kNerEPER},
         {{107, 2}, Tagger::kNer, Tagger::kNerO},
         {{110, 5}, Tagger::kNer, Tagger::kNerO},
         {{116, 2}, Tagger::kNer, Tagger::kNerO},
         {{119, 3}, Tagger::kNer, Tagger::kNerO},
         {{123, 7}, Tagger::kNer, Tagger::kNerO},
         {{130, 2}, Tagger::kNer, Tagger::kNerO},
         {{133, 3}, Tagger::kNer, Tagger::kNerO},
         {{137, 8}, Tagger::kNer, Tagger::kNerO},
         {{146, 2}, Tagger::kNer, Tagger::kNerO},
         {{149, 11}, Tagger::kNer, Tagger::kNerO},
         {{160, 1}, Tagger::kNer, Tagger::kNerO}};
    EXPECT_EQ(exp[0], tags[0]);
    EXPECT_EQ(exp[1], tags[1]);
    EXPECT_EQ(exp[2], tags[2]);
    EXPECT_EQ(exp[3], tags[3]);
    EXPECT_EQ(exp[4], tags[4]);
    EXPECT_EQ(exp[5], tags[5]);
    EXPECT_EQ(exp[6], tags[6]);
    EXPECT_EQ(exp[7], tags[7]);
    EXPECT_EQ(exp[8], tags[8]);
    EXPECT_EQ(exp[9], tags[9]);
    EXPECT_EQ(exp[10], tags[10]);
    EXPECT_EQ(exp[11], tags[11]);
    EXPECT_EQ(exp[12], tags[12]);
    EXPECT_EQ(exp[13], tags[13]);
    EXPECT_EQ(exp[14], tags[14]);
    EXPECT_EQ(exp[15], tags[15]);
    EXPECT_EQ(exp[16], tags[16]);
    EXPECT_EQ(exp[17], tags[17]);
    EXPECT_EQ(exp[18], tags[18]);
    EXPECT_EQ(exp[19], tags[19]);
    EXPECT_EQ(exp[20], tags[20]);
    EXPECT_EQ(exp[21], tags[21]);
    EXPECT_EQ(exp[22], tags[22]);
    EXPECT_EQ(exp[23], tags[23]);
    EXPECT_EQ(exp[24], tags[24]);
    EXPECT_EQ(exp[25], tags[25]);
    EXPECT_EQ(exp[26], tags[26]);
    EXPECT_EQ(exp[27], tags[27]);

    EntityIndex index;
    vector<Tagger::Tag> entities = extractor.Extract(text, &index);
    exp = {{{15, 6}, Tagger::kNer, Tagger::kNerSORG},
           {{94, 3}, Tagger::kNer, Tagger::kNerBPER},
           {{98, 8}, Tagger::kNer, Tagger::kNerEPER}};
    EXPECT_EQ(exp, entities);
    vector<EntityIndex::Item> google_items = {{1.0f}};
    vector<EntityIndex::Item> kurzweil_items = {{1.0f}};
    EXPECT_EQ(google_items, index.Items({"google", Entity::kOrganizationType}));
    EXPECT_EQ(kurzweil_items,
              index.Items({"ray kurzweil", Entity::kPersonType}));
  }
  {
    string text = "Mozilla Corporation and Microsoft are companies both ";
    text += "founded by the notorious Isaac M. Widdlediggle.";
    vector<Tagger::Tag> tags = tagger.Tags(text);
    vector<Tagger::Tag> exp =
        {{{0, 7}, Tagger::kNer, Tagger::kNerBORG},
         {{8, 11}, Tagger::kNer, Tagger::kNerEORG},
         {{20, 3}, Tagger::kNer, Tagger::kNerO},
         {{24, 9}, Tagger::kNer, Tagger::kNerSORG},
         {{34, 3}, Tagger::kNer, Tagger::kNerO},
         {{38, 9}, Tagger::kNer, Tagger::kNerO},
         {{48, 4}, Tagger::kNer, Tagger::kNerO},
         {{53, 7}, Tagger::kNer, Tagger::kNerO},
         {{61, 2}, Tagger::kNer, Tagger::kNerO},
         {{64, 3}, Tagger::kNer, Tagger::kNerO},
         {{68, 9}, Tagger::kNer, Tagger::kNerO},
         {{78, 5}, Tagger::kNer, Tagger::kNerBPER},
         {{84, 2}, Tagger::kNer, Tagger::kNerIPER},
         {{87, 12}, Tagger::kNer, Tagger::kNerEPER},
         {{99, 1}, Tagger::kNer, Tagger::kNerO}};
    EXPECT_EQ(exp[0], tags[0]);
    EXPECT_EQ(exp[1], tags[1]);
    EXPECT_EQ(exp[2], tags[2]);
    EXPECT_EQ(exp[3], tags[3]);
    EXPECT_EQ(exp[4], tags[4]);
    EXPECT_EQ(exp[5], tags[5]);
    EXPECT_EQ(exp[6], tags[6]);
    EXPECT_EQ(exp[7], tags[7]);
    EXPECT_EQ(exp[8], tags[8]);
    EXPECT_EQ(exp[9], tags[9]);
    EXPECT_EQ(exp[10], tags[10]);
    EXPECT_EQ(exp[11], tags[11]);
    EXPECT_EQ(exp[12], tags[12]);
    EXPECT_EQ(exp[13], tags[13]);
    EXPECT_EQ(exp[14], tags[14]);

    EntityIndex index;
    vector<Tagger::Tag> entities = extractor.Extract(text, &index);
    exp = {{{0, 7}, Tagger::kNer, Tagger::kNerBORG},
           {{8, 11}, Tagger::kNer, Tagger::kNerEORG},
           {{24, 9}, Tagger::kNer, Tagger::kNerSORG},
           {{78, 5}, Tagger::kNer, Tagger::kNerBPER},
           {{84, 2}, Tagger::kNer, Tagger::kNerIPER},
           {{87, 12}, Tagger::kNer, Tagger::kNerEPER}};
    EXPECT_EQ(exp, entities);
    vector<EntityIndex::Item> mozilla_items = {{1.0f}};
    vector<EntityIndex::Item> microsoft_items = {{1.0f}};
    vector<EntityIndex::Item> isaac_items = {{1.0f}};
    EXPECT_EQ(mozilla_items,
              index.Items({"mozilla corporation", Entity::kOrganizationType}));
    EXPECT_EQ(microsoft_items,
              index.Items({"microsoft", Entity::kOrganizationType}));
    EXPECT_EQ(isaac_items,
              index.Items({"isaac m. widdlediggle", Entity::kPersonType}));
  }
  {
    string text = "Dick, Asimov, Carroll and Carroll were authors.";
    EntityIndex index;
    vector<Tagger::Tag> entities = extractor.Extract(text, &index);
    vector<Tagger::Tag> exp = {{{0, 4}, Tagger::kNer, Tagger::kNerSPER},
           {{6, 6}, Tagger::kNer, Tagger::kNerSPER},
           {{14, 7}, Tagger::kNer, Tagger::kNerSPER},
           {{26, 7}, Tagger::kNer, Tagger::kNerSPER}};
    EXPECT_EQ(exp, entities);
    vector<EntityIndex::Item> dick_items = {{1.0f}};
    vector<EntityIndex::Item> asimov_items = {{1.0f}};
    vector<EntityIndex::Item> carroll_items = {{1.0f}, {2.0f}};
    EXPECT_EQ(dick_items,
              index.Items({"dick", Entity::kPersonType}));
    EXPECT_EQ(asimov_items,
              index.Items({"asimov", Entity::kPersonType}));
    EXPECT_EQ(carroll_items,
              index.Items({"carroll", Entity::kPersonType}));
  }
}
