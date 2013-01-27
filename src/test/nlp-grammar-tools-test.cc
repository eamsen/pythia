// Copyright 2013 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <unordered_set>
#include "../nlp/grammar-tools.h"

using std::vector;
using std::unordered_set;
using std::set;
using std::string;
using std::cout;
using std::endl;
using std::ofstream;

using ::testing::ElementsAre;
using ::testing::Contains;
using ::testing::Not;

using namespace pyt::nlp;  // NOLINT

TEST(GrammarToolsTest, ConsonantsAndVowels) {
  EXPECT_FALSE(IsConsonant('a'));
  EXPECT_TRUE(IsVowel('a'));
  EXPECT_FALSE(IsConsonant('e'));
  EXPECT_TRUE(IsVowel('e'));
  EXPECT_FALSE(IsConsonant('i'));
  EXPECT_TRUE(IsVowel('i'));
  EXPECT_FALSE(IsConsonant('o'));
  EXPECT_TRUE(IsVowel('o'));
  EXPECT_FALSE(IsConsonant('u'));
  EXPECT_TRUE(IsVowel('u'));

  EXPECT_TRUE(IsConsonant('b'));
  EXPECT_FALSE(IsVowel('b'));
  EXPECT_TRUE(IsConsonant('c'));
  EXPECT_FALSE(IsVowel('c'));
  EXPECT_TRUE(IsConsonant('d'));
  EXPECT_FALSE(IsVowel('d'));
  EXPECT_TRUE(IsConsonant('f'));
  EXPECT_FALSE(IsVowel('f'));
  EXPECT_TRUE(IsConsonant('g'));
  EXPECT_FALSE(IsVowel('g'));
  EXPECT_TRUE(IsConsonant('h'));
  EXPECT_FALSE(IsVowel('h'));
  EXPECT_TRUE(IsConsonant('j'));
  EXPECT_FALSE(IsVowel('j'));
  EXPECT_TRUE(IsConsonant('k'));
  EXPECT_FALSE(IsVowel('k'));
  EXPECT_TRUE(IsConsonant('l'));
  EXPECT_FALSE(IsVowel('l'));
  EXPECT_TRUE(IsConsonant('m'));
  EXPECT_FALSE(IsVowel('m'));
  EXPECT_TRUE(IsConsonant('n'));
  EXPECT_FALSE(IsVowel('n'));
  EXPECT_TRUE(IsConsonant('p'));
  EXPECT_FALSE(IsVowel('p'));
  EXPECT_TRUE(IsConsonant('q'));
  EXPECT_FALSE(IsVowel('q'));
  EXPECT_TRUE(IsConsonant('r'));
  EXPECT_FALSE(IsVowel('r'));
  EXPECT_TRUE(IsConsonant('s'));
  EXPECT_FALSE(IsVowel('s'));
  EXPECT_TRUE(IsConsonant('t'));
  EXPECT_FALSE(IsVowel('t'));
  EXPECT_TRUE(IsConsonant('v'));
  EXPECT_FALSE(IsVowel('v'));
  EXPECT_TRUE(IsConsonant('w'));
  EXPECT_FALSE(IsVowel('w'));
  EXPECT_TRUE(IsConsonant('x'));
  EXPECT_FALSE(IsVowel('x'));
  EXPECT_TRUE(IsConsonant('z'));
  EXPECT_FALSE(IsVowel('z'));
}

TEST(GrammarToolsTest, SingularForms) {
  string singular = "book";
  vector<string> result_vec = SingularForms("books");
  unordered_set<string> result(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "bedroom";
  result_vec = SingularForms("bedrooms");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "bus";
  result_vec = SingularForms("buses");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "church";
  result_vec = SingularForms("churches");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "box";
  result_vec = SingularForms("boxes");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "buzz";
  result_vec = SingularForms("buzzes");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "city";
  result_vec = SingularForms("cities");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "baby";
  result_vec = SingularForms("babies");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "country";
  result_vec = SingularForms("countries");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "Kennedy";
  result_vec = SingularForms("Kennedys");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "potato";
  result_vec = SingularForms("potatoes");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "hero";
  result_vec = SingularForms("heroes");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 

  singular = "echo";
  result_vec = SingularForms("echoes");
  result = unordered_set<string>(result_vec.begin(), result_vec.end());
  EXPECT_TRUE(result.count(singular)); 
}
