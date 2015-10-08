#include "core/pattern/pattern_matcher.h"

#include <string>

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/pattern/field_pattern.h"

using namespace std;

static SmallIntSet toSmallIntSet(initializer_list<char> cs)
{
    SmallIntSet s;
    for (char c : cs) {
        CHECK('A' <= c && c <= 'Z') << c;
        s.set(c - 'A');
    }
    return s;
}

static PatternMatchResult match(const FieldPattern& pattern, const CoreField& cf, bool ignoresMustVar = false)
{
    PatternMatcher matcher;
    return matcher.match(pattern, cf, ignoresMustVar);
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

    EXPECT_EQ(PatternMatchResult(true, FieldBits("......"),
                                 toSmallIntSet({'A', 'B', 'C', 'D', 'X'})),
              match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "111.1."
                                     "111111"
                                     "111111"),
                                 toSmallIntSet({})),
              match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "111.1."
                                     "111111"
                                     "111111"),
                                 toSmallIntSet({})),
              match(pattern, f2));
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

    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "1..1.1"
                                     "1111.1"),
                                 toSmallIntSet({'D', 'E', 'X', 'Z'})),
              match(pattern, f));
}

TEST(PatternMatcherTest, matchWithMust)
{
    FieldPattern pattern("AAABBB");
    pattern.setMustVar(1, 1);
    pattern.setMustVar(6, 1);

    CoreField f0("RRRBBB");
    CoreField f1("RRRBB.");

    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("111111"),
                                 toSmallIntSet({})),
              match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("111111"),
                                 toSmallIntSet({})),
              match(pattern, f0, true));

    EXPECT_EQ(PatternMatchResult(), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("11111."),
                                 toSmallIntSet({})),
              match(pattern, f1, true));
}

TEST(PatternMatcherTest, unusedVariables)
{
    FieldPattern pattern(
        "AAABBB");

    CoreField f("Y     ");

    PatternMatchResult result = match(pattern, f);

    SmallIntSet expected(toSmallIntSet({'B'}));
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(expected, result.unusedVariables);
}
