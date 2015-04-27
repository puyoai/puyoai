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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, toSmallIntSet({'A'})), match(pattern, f0));
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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, toSmallIntSet({'A', 'B', 'C', 'D', 'X'})), match(pattern, f0));
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

    EXPECT_EQ(PatternMatchResult(true, 8, 8, 0, toSmallIntSet({'D', 'E', 'X', 'Z'})), match(pattern, f));
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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, toSmallIntSet({'A', 'B', 'C'})), match(pattern, f0));
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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, toSmallIntSet({'A', 'B', 'C'})), match(pattern, f0));
    EXPECT_EQ(PatternMatchResult(), match(pattern, f1));
    EXPECT_EQ(PatternMatchResult(true, 4, 4, 1, toSmallIntSet({'C'})), match(pattern, f2));
    EXPECT_EQ(PatternMatchResult(true, 3, 3, 1, toSmallIntSet({'C'})), match(pattern, f3));
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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, toSmallIntSet({'A', 'B'})), match(pattern, f0, true));
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

    SmallIntSet expected(toSmallIntSet({'B'}));
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(expected, result.unusedVariables);
}

TEST(PatternMatcherTest, complement1)
{
    FieldPattern pattern(
        "..BC.."
        "AAAB.."
        "BBBCCC");

    CoreField cf(
        "......"
        "..YB.."
        "BBBG..");

    CoreField expected(
        "..BG.."
        "YYYB.."
        "BBBGGG");

    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.complement(pattern, cf, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_EQ(expected, cf) << cf.toDebugString();
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

    // Since we cannot complement (3, 3), so this pattern should not match.
    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_FALSE(matcher.complement(pattern, cf, &cpl).success);
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

    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_FALSE(matcher.complement(pattern, cf, &cpl).success);
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

    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.complement(pattern, cf, &cpl).success);
}

TEST(PatternMatcherTest, complement5)
{
    FieldPattern pattern(
        "ABC..."
        "AABCC."
        "BBC@@.");

    EXPECT_EQ(PatternType::ALLOW_FILLING_OJAMA, pattern.type(4, 1));

    CoreField cf(
        "Y....."
        "Y....."
        "GGB...");

    CoreField expected(
        "YGB..."
        "YYGBB."
        "GGBOO.");

    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.complement(pattern, cf, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_EQ(expected, cf) << expected.toDebugString() << '\n' << cf.toDebugString();
}

TEST(PatternMatcherTest, complement6)
{
    FieldPattern pattern(
        "ABC..."
        "AABCC."
        "BBC@@.");

    EXPECT_EQ(PatternType::ALLOW_FILLING_OJAMA, pattern.type(4, 1));

    CoreField cf(
        "Y....."
        "Y....."
        "GG....");

    CoreField expected1(
        "YGB..."
        "YYGBB."
        "GGBOO.");

    CoreField expected2(
        "YGR..."
        "YYGRR."
        "GGROO.");

    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.complement(pattern, cf, 1, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_TRUE(cf == expected1 || cf == expected2) << cf.toDebugString();
}

TEST(PatternMatcherTest, complementWithAllow1)
{
    FieldPattern pattern(
        ".ab..."
        ".AB..."
        "ABC..."
        "ABC..."
        "ABCC..");

    CoreField cf(
        "YGB..."
        "YGB..."
        "YGB...");

    CoreField expected(
        ".YG..."
        "YGB..."
        "YGB..."
        "YGBB..");

    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.complement(pattern, cf, 1, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_TRUE(expected == cf);
}

TEST(PatternMatcherTest, complementWithAllow2)
{
    FieldPattern pattern(
        ".ab..."
        ".AB..."
        "ABC..."
        "ABC..."
        "ABCC..");

    CoreField cf(
        ".Y...."
        ".Y...."
        "YGB..."
        "YGB..."
        "YGB...");

    CoreField expected(
        ".Y...."
        ".YG..."
        "YGB..."
        "YGB..."
        "YGBB..");


    ColumnPuyoList cpl;
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.complement(pattern, cf, 1, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_TRUE(expected == cf);
}
