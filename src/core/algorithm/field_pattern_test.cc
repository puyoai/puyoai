#include "core/algorithm/field_pattern.h"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(FieldPatternTest, initial)
{
    FieldPattern pattern;
    for (int i = 1; i <= 6; ++i) {
        EXPECT_EQ(0, pattern.height(i));
    }
}

TEST(FieldPatternTest, constructor1)
{
    FieldPattern pattern(
        ".....B"
        "....BB"
        "...BBB"
        "..BBBB"
        ".BBBBB"
        "AAABBC");

    EXPECT_EQ(1, pattern.height(1));
    EXPECT_EQ(2, pattern.height(2));
    EXPECT_EQ(3, pattern.height(3));
    EXPECT_EQ(4, pattern.height(4));
    EXPECT_EQ(5, pattern.height(5));
    EXPECT_EQ(6, pattern.height(6));

    EXPECT_EQ('A', pattern.variable(1, 1));
    EXPECT_EQ('B', pattern.variable(2, 2));
    EXPECT_EQ('C', pattern.variable(6, 1));
    EXPECT_EQ(' ', pattern.variable(1, 2));
}

TEST(FieldPatternTest, constructor2)
{
    FieldPattern pattern(vector<string> {
        ".....B",
        "....BB",
        "...BBB",
        "..BBBB",
        ".BBBBB",
        "AAABBC"
    });

    EXPECT_EQ(1, pattern.height(1));
    EXPECT_EQ(2, pattern.height(2));
    EXPECT_EQ(3, pattern.height(3));
    EXPECT_EQ(4, pattern.height(4));
    EXPECT_EQ(5, pattern.height(5));
    EXPECT_EQ(6, pattern.height(6));

    EXPECT_EQ('A', pattern.variable(1, 1));
    EXPECT_EQ('B', pattern.variable(2, 2));
    EXPECT_EQ('C', pattern.variable(6, 1));
    EXPECT_EQ(' ', pattern.variable(1, 2));
}

TEST(FieldPatternTest, varCount)
{
    FieldPattern pattern1("AAA...");
    FieldPattern pattern2("..AAAB");
    FieldPattern pattern3(".*ABBB");

    EXPECT_EQ(3, pattern1.numVariables());
    EXPECT_EQ(4, pattern2.numVariables());
    EXPECT_EQ(4, pattern3.numVariables()); // We don't count *

    {
        FieldPattern pattern;
        ASSERT_TRUE(FieldPattern::merge(pattern1, pattern2, &pattern));
        EXPECT_EQ(6, pattern.numVariables());
    }
    {
        FieldPattern pattern;
        ASSERT_TRUE(FieldPattern::merge(pattern1, pattern2, &pattern));
        EXPECT_EQ(6, pattern.numVariables());
    }
}

TEST(FieldPatternTest, fillSameVariablePositions)
{
    FieldPattern pattern(
        "C....."
        "CAA..."
        "CCDAAA"
        "AAADDD");

    FieldBitField checked;
    Position positionQueue[FieldConstant::WIDTH * FieldConstant::HEIGHT];
    Position* p = pattern.fillSameVariablePositions(1, 2, 'C', positionQueue, &checked);
    EXPECT_EQ(4, p - positionQueue);

    std::sort(positionQueue, p);
    EXPECT_EQ(Position(1, 2), positionQueue[0]);
    EXPECT_EQ(Position(1, 3), positionQueue[1]);
    EXPECT_EQ(Position(1, 4), positionQueue[2]);
    EXPECT_EQ(Position(2, 2), positionQueue[3]);
}

TEST(FieldPatternTest, complement1)
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
    EXPECT_TRUE(pattern.complement(cf, false, &cpl));
    for (const auto& cp : cpl) {
        cf.dropPuyoOn(cp.x, cp.color);
    }
    EXPECT_EQ(expected, cf) << cf.toDebugString();
}

TEST(FieldPatternTest, complement2)
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
    EXPECT_FALSE(pattern.complement(cf, false, &cpl));
}

TEST(FieldPatternTest, complement3)
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
    EXPECT_FALSE(pattern.complement(cf, false, &cpl));
}

TEST(FieldPatternTest, complement4)
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
    EXPECT_TRUE(pattern.complement(cf, false, &cpl));
}

TEST(FieldPatternTest, complement5)
{
    FieldPattern pattern(
        "ABC..."
        "AABCC."
        "BBC...");

    CoreField cf(
        "Y....."
        "Y....."
        "GGB...");

    CoreField expected(
        "YGB..."
        "YYGBB."
        "GGBOO.");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pattern.complement(cf, true, &cpl));
    for (const ColumnPuyo& cp : cpl) {
        cf.dropPuyoOn(cp.x, cp.color);
    }

    EXPECT_EQ(expected, cf) << expected.toDebugString() << '\n' << cf.toDebugString();
}
