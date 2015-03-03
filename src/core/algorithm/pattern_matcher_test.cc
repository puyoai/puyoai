#include "core/algorithm/pattern_matcher.h"

#include <string>

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/algorithm/field_pattern.h"

using namespace std;

static PatternMatchResult match(const FieldPattern& pattern, const CoreField& cf, bool ignoresMustVar = false)
{
    PatternMatcher matcher;
    return matcher.match(pattern, cf, ignoresMustVar);
}

TEST(PatternMatcherTest, match1)
{
    FieldPattern pattern(
        "AAA...");

    CoreField f0;
    CoreField f1("RRR   ");
    CoreField f2("R     ");
    CoreField f3("R R   ");

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, {'A'}), match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true, 3, 3, 0), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true, 1, 1, 0), match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true, 2, 2, 0), match(pattern, f3));
}

TEST(PatternMatcherTest, match2)
{
    FieldPattern pattern(
        "BAD.C."
        "BBADDD"
        "AACCCX");

    CoreField f0;

    CoreField f1(
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    CoreField f2(
        "BYB R "
        "BBYBBB"
        "YYRRRG");

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, {'A', 'B', 'C', 'D', 'X'}), match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true, 16, 16, 0), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true, 16, 16, 0), match(pattern, f2));
}

TEST(PatternMatcherTest, match3)
{
    FieldPattern pattern(
        "b....."
        "BEExy."
        "DDEXYy"
        "BACXYZ"
        "BBACXY"
        "AACCXY");

    CoreField f(
        "R..B.B"
        "YYBB.B");

    EXPECT_EQ(PatternMatchResult(true, 8, 8, 0, {'D', 'E', 'X', 'Z'}), match(pattern, f));
}

TEST(PatternMatcherTest, matchWithStar)
{
    FieldPattern pattern(
        ".***CC"
        ".AABBB");

    CoreField f0;

    CoreField f1(
        ".YYYGG"
        ".RRBBB");

    CoreField f2(
        ".B...."
        ".BBYYY"
        ".RRBBB");

    CoreField f3(
        ".GGGYY"
        ".RRBBB");

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, {'A', 'B', 'C'}), match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true, 7, 7, 0), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true, 7, 7, 0), match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true, 7, 7, 0), match(pattern, f3));
}

TEST(PatternMatcherTest, matchWithAllowing)
{
    FieldPattern pattern(
        "..C..."
        "AAaBB.");

    CoreField f0;

    CoreField f1(
        "RRBBB.");

    CoreField f2(
        "RRRBB.");

    CoreField f3(
        "R.RBB.");

    CoreField f4(
        "..R..."
        "RRYRR.");

    CoreField f5(
        "RRRRR.");

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, {'A', 'B', 'C'}), match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true, 4, 4, 1, {'C'}), match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true, 3, 3, 1, {'C'}), match(pattern, f3));
    EXPECT_EQ(PatternMatchResult(true, 5, 5, 0), match(pattern, f4));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f5));
}

TEST(PatternMatcherTest, matchWithMust)
{
    FieldPattern pattern("AAABBB");
    pattern.setType(1, 1, PatternType::MUST_VAR);
    pattern.setType(6, 1, PatternType::MUST_VAR);

    CoreField f0;
    CoreField f1(
        "RRRBBB");
    CoreField f2(
        "RRRBB.");
    CoreField f3(
        "R.RB.B");
    CoreField f4(
        "R....."
        "RR.BBB");
    CoreField f5(
        "RRRRRR");

    EXPECT_EQ(PatternMatchResult(), match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true, 6, 6, 0), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true, 4, 4, 0), match(pattern, f3));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f4));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f5));

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, {'A', 'B'}), match(pattern, f0, true));
    EXPECT_EQ(PatternMatchResult(true, 6, 6, 0), match(pattern, f1, true));
    EXPECT_EQ(PatternMatchResult(true, 5, 5, 0), match(pattern, f2, true));
    EXPECT_EQ(PatternMatchResult(true, 4, 4, 0), match(pattern, f3, true));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f4, true));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f5, true));
}

TEST(PatternMatcherTest, unmatch1)
{
    FieldPattern pattern(
        "BAD.C."
        "BBADDD"
        "AACCCX");

    CoreField f1(
        "B     "
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    CoreField f2(
        "    R "
        "BYB R "
        "BBYBBB"
        "YYRRRG");

    CoreField f3(
        "BYO R "
        "BBYOOO"
        "YYRRRY");

    CoreField f4(
        "BRB R "
        "BBRBBB"
        "RRRRRY");

    EXPECT_EQ(PatternMatchResult(), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f3));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f4));
}

TEST(PatternMatcherTest, unmatch2)
{
    FieldPattern pattern(
        "..AAA.");

    CoreField f1(" B B  ");

    EXPECT_EQ(PatternMatchResult(), match(pattern, f1));
}

TEST(PatternMatcherTest, unmatch3)
{
    FieldPattern pattern(
        "AAABBB");

    CoreField f("Y    Y");

    EXPECT_EQ(PatternMatchResult(), match(pattern, f));
}

TEST(PatternMatcherTest, unusedVariables)
{
    FieldPattern pattern(
        "AAABBB");

    CoreField f("Y     ");

    PatternMatchResult result = match(pattern, f);

    vector<char> expected { 'B' };
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(expected, result.unusedVariables);
}
