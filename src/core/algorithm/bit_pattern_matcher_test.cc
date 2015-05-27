#include "bit_pattern_matcher.h"

#include <gtest/gtest.h>

#include "core/bit_field.h"

#include "bit_field_pattern.h"

TEST(BitPatternMatcherTest, match1)
{
    BitFieldPattern pattern(
        "AAABBB");

    BitField bf("RRRBBB");

    BitPatternMatcher matcher;
    EXPECT_TRUE(matcher.match(pattern, bf));
}

TEST(BitPatternMatcherTest, match2)
{
    BitFieldPattern pattern(
        "BAD.C."
        "BBADDD"
        "AACCCX");

    BitField bf0;
    BitField bf1(
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");
    BitField bf2(
        "BYB R "
        "BBYBBB"
        "YYRRRG");

    EXPECT_TRUE(BitPatternMatcher().match(pattern, bf0));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, bf1));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, bf2));
}

TEST(BitPatternMatcherTest, match3)
{
    BitFieldPattern pattern(
        "b....."
        "BEExy."
        "DDEXYy"
        "BACXYZ"
        "BBACXY"
        "AACCXY");

    BitField bf(
        "R..B.B"
        "YYBB.B");

    EXPECT_TRUE(BitPatternMatcher().match(pattern, bf));
}

TEST(BitPatternMatcherTest, match4)
{
    BitFieldPattern pattern(
        "..C..."
        "AAaBB.");

    BitField f0;

    BitField f1(
        "RRBBB.");

    BitField f2(
        "RRRBB.");

    BitField f3(
        "R.RBB.");

    BitField f4(
        "..R..."
        "RRYRR.");

    BitField f5(
        "RRRRR.");

    EXPECT_TRUE(BitPatternMatcher().match(pattern, f0));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, f2));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, f3));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, f4));

    EXPECT_FALSE(BitPatternMatcher().match(pattern, f1));
    EXPECT_FALSE(BitPatternMatcher().match(pattern, f5));
}

TEST(BitPatternMatcherTest, matchWithStar)
{
    BitFieldPattern pattern(
        ".***CC"
        ".AABBB");

    BitField f0;

    BitField f1(
        ".YYYGG"
        ".RRBBB");

    BitField f2(
        ".B...."
        ".BBYYY"
        ".RRBBB");

    BitField f3(
        ".GGGYY"
        ".RRBBB");

    EXPECT_TRUE(BitPatternMatcher().match(pattern, f0));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, f1));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, f2));
    EXPECT_TRUE(BitPatternMatcher().match(pattern, f3));
}

TEST(BitPatternMatcherTest, unmatch2)
{
    BitFieldPattern pattern(
        "..AAA.");

    BitField f1(" B B  ");

    EXPECT_FALSE(BitPatternMatcher().match(pattern, f1));
}
