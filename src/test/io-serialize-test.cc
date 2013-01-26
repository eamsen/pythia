// Copyright 2013 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
#include <list>
#include <sstream>
#include <limits>
#include "../io/serialize.h"

using std::vector;
using std::unordered_map;
using std::map;
using std::unordered_set;
using std::set;
using std::string;
using std::cout;
using std::endl;
using std::stringstream;
using std::numeric_limits;

using ::testing::ElementsAre;
using ::testing::Contains;
using ::testing::Not;

using namespace pyt::io;  // NOLINT

TEST(SerializeTest, pod) {
  stringstream stream;
  {
    int v = 0;
    int r;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = 1112991;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = -1112991;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = numeric_limits<int>::max();
    for (int i = 0; i < 99; ++i) {
      Write(v--, stream);
    }
    v = numeric_limits<int>::max();
    for (int i = 0; i < 99; ++i) {
      Read(stream, &r);
      ASSERT_EQ(v--, r);
    }
  }
  {
    size_t v = 1112991;
    size_t r;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);
  }
}

TEST(SerializeTest, string) {
  stringstream stream;
  string v;
  string r;
  Write(v, stream);
  Read(stream, &r);
  EXPECT_EQ(v, r);

  v = "fantastic";
  r = "";
  Write(v, stream);
  Read(stream, &r);
  EXPECT_EQ(v, r);

  v = "a";
  for (int i = 0; i < 99; ++i) {
    Write(v, stream);
  }
  v = "";
  r = "";
  for (int i = 0; i < 99; ++i) {
    v += "a";
    Read(stream, &r);
    ASSERT_EQ(v, r);
  }
}

TEST(SerializeTest, vector) {
  stringstream stream;
  {
    vector<int> v;
    vector<int> r;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = {0, 1, 2, 3, 4, 5};
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    for (int i = 0; i < 99; ++i) {
      v = vector<int>(i, 12);
      Write(v, stream);
    }
    for (int i = 0; i < 99; ++i) {
      v = vector<int>(i, 12);
      r = {};
      Read(stream, &r);
      ASSERT_EQ(v, r);
    }
  }
}

TEST(SerializeTest, unordered_map) {
  stringstream stream;
  {
    unordered_map<int, int> v;
    unordered_map<int, int> r;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = {{0, 1}, {1, 2}, {2, 3}};
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    for (int i = 0; i < 99; ++i) {
      v = {{i, 12}, {i + 1, 13}};
      Write(v, stream);
    }
    for (int i = 0; i < 99; ++i) {
      v = {{i, 12}, {i + 1, 13}};
      r = {};
      Read(stream, &r);
      ASSERT_EQ(v, r);
    }
  }
}

TEST(SerializeTest, map) {
  stringstream stream;
  {
    map<int, int> v;
    map<int, int> r;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = {{0, 1}, {1, 2}, {2, 3}};
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    for (int i = 0; i < 99; ++i) {
      v = {{i, 12}, {i + 1, 13}};
      Write(v, stream);
    }
    for (int i = 0; i < 99; ++i) {
      v = {{i, 12}, {i + 1, 13}};
      r = {};
      Read(stream, &r);
      ASSERT_EQ(v, r);
    }
  }
}

TEST(SerializeTest, unordered_set) {
  stringstream stream;
  {
    unordered_set<int> v;
    unordered_set<int> r;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = {0, 1, 2, 3};
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    for (int i = 0; i < 99; ++i) {
      v = {i, i + 1, i + 2};
      Write(v, stream);
    }
    for (int i = 0; i < 99; ++i) {
      v = {i, i + 1, i + 2};
      r = {};
      Read(stream, &r);
      ASSERT_EQ(v, r);
    }
  }
}

TEST(SerializeTest, set) {
  stringstream stream;
  {
    set<int> v;
    set<int> r;
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    v = {0, 1, 2, 3};
    Write(v, stream);
    Read(stream, &r);
    EXPECT_EQ(v, r);

    for (int i = 0; i < 99; ++i) {
      v = {i, i + 1, i + 2};
      Write(v, stream);
    }
    for (int i = 0; i < 99; ++i) {
      v = {i, i + 1, i + 2};
      r = {};
      Read(stream, &r);
      ASSERT_EQ(v, r);
    }
  }
}
