#include "core/algorithm/pattern_matcher.h"

#include <string>

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/algorithm/pattern_field.h"

static PatternMatchResult match(const PatternField& pf, const CoreField& cf, bool ignoresMustVar = false)
{
    PatternMatcher matcher;
    return matcher.match(pf, cf, ignoresMustVar);
}

TEST(PatternMatcherTest, match1)
{
    PatternField pf(
        "AAA...");

    CoreField f0;
    CoreField f1("RRR   ");
    CoreField f2("R     ");
    CoreField f3("R R   ");

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0), match(pf, f0));
    EXPECT_EQ(PatternMatchResult(true, 3, 3, 0), match(pf, f1));
    EXPECT_EQ(PatternMatchResult(true, 1, 1, 0), match(pf, f2));
    EXPECT_EQ(PatternMatchResult(true, 2, 2, 0), match(pf, f3));
}

TEST(PatternMatcherTest, match2)
{
    PatternField pf(
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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0), match(pf, f0));
    EXPECT_EQ(PatternMatchResult(true, 16, 16, 0), match(pf, f1));
    EXPECT_EQ(PatternMatchResult(true, 16, 16, 0), match(pf, f2));
}

TEST(PatternMatcherTest, match3)
{
    PatternField pf(
        "b....."
        "BEExy."
        "DDEXYy"
        "BACXYZ"
        "BBACXY"
        "AACCXY");

    CoreField f(
        "R..B.B"
        "YYBB.B");

    EXPECT_EQ(PatternMatchResult(true, 8, 8, 0), match(pf, f));
}

TEST(PatternMatcherTest, matchWithStar)
{
    PatternField pf(
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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0), match(pf, f0));
    EXPECT_EQ(PatternMatchResult(true, 7, 7, 0), match(pf, f1));
    EXPECT_EQ(PatternMatchResult(true, 7, 7, 0), match(pf, f2));
    EXPECT_EQ(PatternMatchResult(true, 7, 7, 0), match(pf, f3));
}

TEST(PatternMatcherTest, matchWithAllowing)
{
    PatternField pf(
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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0), match(pf, f0));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f1));
    EXPECT_EQ(PatternMatchResult(true, 4, 4, 1), match(pf, f2));
    EXPECT_EQ(PatternMatchResult(true, 3, 3, 1), match(pf, f3));
    EXPECT_EQ(PatternMatchResult(true, 5, 5, 0), match(pf, f4));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f5));
}

TEST(PatternMatcherTest, matchWithMust)
{
    PatternField pf("AAABBB");
    pf.setType(1, 1, PatternType::MUST_VAR);
    pf.setType(6, 1, PatternType::MUST_VAR);

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

    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f0));
    EXPECT_EQ(PatternMatchResult(true, 6, 6, 0), match(pf, f1));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f2));
    EXPECT_EQ(PatternMatchResult(true, 4, 4, 0), match(pf, f3));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f4));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f5));

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0), match(pf, f0, true));
    EXPECT_EQ(PatternMatchResult(true, 6, 6, 0), match(pf, f1, true));
    EXPECT_EQ(PatternMatchResult(true, 5, 5, 0), match(pf, f2, true));
    EXPECT_EQ(PatternMatchResult(true, 4, 4, 0), match(pf, f3, true));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f4, true));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f5, true));
}

TEST(PatternMatcherTest, unmatch1)
{
    PatternField pf(
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

    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f1));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f2));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f3));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f4));
}

TEST(PatternMatcherTest, unmatch2)
{
    PatternField pf(
        "..AAA.");

    CoreField f1(" B B  ");

    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f1));
}

TEST(PatternMatcherTest, unmatch3)
{
    PatternField pf(
        "AAABBB");

    CoreField f("Y    Y");

    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), match(pf, f));
}
