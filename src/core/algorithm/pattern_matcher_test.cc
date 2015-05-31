#include "core/algorithm/pattern_matcher.h"

#include <string>

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/algorithm/field_pattern.h"

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

TEST(PatternMatcherTest, match1)
{
    FieldPattern pattern(
        "AAA...");

    CoreField f0;
    CoreField f1("RRR   ");
    CoreField f2("R     ");
    CoreField f3("R R   ");

    EXPECT_EQ(PatternMatchResult(true, FieldBits("......"), FieldBits("......"), toSmallIntSet({'A'})), match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true, FieldBits("111..."), FieldBits("......"), toSmallIntSet({})), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true, FieldBits("1....."), FieldBits("......"), toSmallIntSet({})), match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true, FieldBits("1.1..."), FieldBits("......"), toSmallIntSet({})), match(pattern, f3));
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

    EXPECT_EQ(PatternMatchResult(true, FieldBits("......"), FieldBits("......"),
                                 toSmallIntSet({'A', 'B', 'C', 'D', 'X'})),
              match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "111.1."
                                     "111111"
                                     "111111"),
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "111.1."
                                     "111111"
                                     "111111"),
                                 FieldBits("......"),
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
                                 FieldBits("......"),
                                 toSmallIntSet({'D', 'E', 'X', 'Z'})),
              match(pattern, f));
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

    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("......"),
                                 FieldBits("......"),
                                 toSmallIntSet({'A', 'B', 'C'})),
              match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "....11"
                                     ".11111"),
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "....11"
                                     ".11111"),
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "....11"
                                     ".11111"),
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f3));
}

TEST(PatternMatcherTest, matchWithAllowing)
{
    FieldPattern pattern(
        "..C..."
        "AAaBB.");

    CoreField f0;

    CoreField f1(
        "RRRBB.");

    CoreField f2(
        "R.RBB.");

    CoreField f3(
        "..R..."
        "RRYRR.");

    CoreField f4(
        "RRBBB.");

    CoreField f5(
        "RRRRR.");

    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("......"),
                                 FieldBits("......"),
                                 toSmallIntSet({'A', 'B', 'C'})),
              match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("11.11."),
                                 FieldBits("..1..."),
                                 toSmallIntSet({'C'})),
              match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("1..11."),
                                 FieldBits("..1..."),
                                 toSmallIntSet({'C'})),
              match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits(
                                     "..1..."
                                     "11.11."),
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f3));

    EXPECT_EQ(PatternMatchResult(), match(pattern, f4));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f5));
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
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("111111"),
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f0, true));

    EXPECT_EQ(PatternMatchResult(), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true,
                                 FieldBits("11111."),
                                 FieldBits("......"),
                                 toSmallIntSet({})),
              match(pattern, f1, true));
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

    SmallIntSet expected(toSmallIntSet({'B'}));
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(expected, result.unusedVariables);
}

TEST(PatternMatcherTest, complement1)
{
    const FieldPattern pattern(
        "..BC.."
        "AAAB.."
        "BBBCCC");

    const CoreField original(
        "......"
        "..YB.."
        "BBBG..");

    const CoreField expected(
        "..BG.."
        "YYYB.."
        "BBBGGG");

    ComplementResult result = PatternMatcher().complement(pattern, original);
    EXPECT_TRUE(result.success);

    CoreField cf(original);
    ASSERT_TRUE(cf.dropPuyoList(result.complementedPuyoList));
    EXPECT_EQ(expected, cf);
}

TEST(PatternMatcherTest, complement2)
{
    FieldPattern pattern(
        "B....."
        "AAA..."
        "BC...."
        "BBC..."
        "CC....");

    CoreField cf(
        "YB...."
        "YYB..."
        "BBGGG.");

    // Since we cannot complement (3, 3), this pattern should not match.
    PatternMatcher matcher;
    EXPECT_FALSE(matcher.complement(pattern, cf).success);
}

TEST(PatternMatcherTest, complement3)
{
    FieldPattern pattern(
        "BA...."
        "AA...."
        "BC...."
        "BBC..."
        "CC....");

    CoreField cf(
        " R    "
        "YY BBB"
        "RRRGGG");

    PatternMatcher matcher;
    EXPECT_FALSE(matcher.complement(pattern, cf).success);
}

TEST(PatternMatcherTest, complement4)
{
    FieldPattern pattern(
        "....De"
        "ABCDDE"
        "AABCCD"
        "BBCEEE");

    CoreField cf(
        ".....Y"
        "Y....Y"
        "Y..RRB"
        "GG.YYY");

    PatternMatcher matcher;
    EXPECT_TRUE(matcher.complement(pattern, cf).success);
}

TEST(PatternMatcherTest, complement5)
{
    const FieldPattern pattern(
        "ABC..."
        "AABCC."
        "BBC&&.");

    const CoreField original(
        "Y....."
        "Y....."
        "GGB...");

    const CoreField expected(
        "YGB..."
        "YYGBB."
        "GGB&&.");

    ComplementResult result = PatternMatcher().complement(pattern, original);
    EXPECT_TRUE(result.success);

    CoreField cf(original);
    ASSERT_TRUE(cf.dropPuyoList(result.complementedPuyoList));
    EXPECT_EQ(expected, cf);
}

TEST(PatternMatcherTest, complement6)
{
    const FieldPattern pattern(
        "ABC..."
        "AABCC."
        "BBC&&.");

    const CoreField original(
        "Y....."
        "Y....."
        "GG....");

    const CoreField expected1(
        "YGB..."
        "YYGBB."
        "GGB&&.");

    const CoreField expected2(
        "YGR..."
        "YYGRR."
        "GGR&&.");

    ComplementResult result = PatternMatcher().complement(pattern, original, 1);
    EXPECT_TRUE(result.success);

    CoreField cf(original);
    ASSERT_TRUE(cf.dropPuyoList(result.complementedPuyoList));
    EXPECT_TRUE(cf == expected1 || cf == expected2);
}

TEST(PatternMatcherTest, complementWithAllow1)
{
    const FieldPattern pattern(
        ".ab..."
        ".AB..."
        "ABC..."
        "ABC..."
        "ABCC..");

    const CoreField original(
        "YGB..."
        "YGB..."
        "YGB...");

    const CoreField expected(
        ".YG..."
        "YGB..."
        "YGB..."
        "YGBB..");

    ComplementResult result = PatternMatcher().complement(pattern, original, 1);
    EXPECT_TRUE(result.success);

    CoreField cf(original);
    ASSERT_TRUE(cf.dropPuyoList(result.complementedPuyoList));
    EXPECT_EQ(expected, cf);
}

TEST(PatternMatcherTest, complementWithAllow2)
{
    const FieldPattern pattern(
        ".ab..."
        ".AB..."
        "ABC..."
        "ABC..."
        "ABCC..");

    const CoreField original(
        ".Y...."
        ".Y...."
        "YGB..."
        "YGB..."
        "YGB...");

    const CoreField expected(
        ".Y...."
        ".YG..."
        "YGB..."
        "YGB..."
        "YGBB..");

    ComplementResult result = PatternMatcher().complement(pattern, original, 1);
    EXPECT_TRUE(result.success);

    CoreField cf(original);
    ASSERT_TRUE(cf.dropPuyoList(result.complementedPuyoList));
    EXPECT_EQ(expected, cf);
}

TEST(PatternMatcherTest, complementWithPlaceholder1)
{
    const FieldPattern pattern(
        "...B.."
        "AAABB.");

    const CoreField original(
        "......"
        "GGG&B.");

    const CoreField expected(
        "...B.."
        "GGGBB.");

    ComplementResult result = PatternMatcher().complement(pattern, original);
    EXPECT_TRUE(result.success);

    CoreField cf(original);
    cf.removePuyoFrom(4);
    ASSERT_TRUE(cf.dropPuyoList(result.complementedPuyoList));
    EXPECT_EQ(expected, cf);
}
