#include "pattern_field.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(PatternFieldTest, initial)
{
    PatternField pf;
    for (int i = 1; i <= 6; ++i) {
        EXPECT_EQ(0, pf.height(i));
    }
}

TEST(PatternFieldTest, constructor1)
{
    PatternField pf(
        ".....B"
        "....BB"
        "...BBB"
        "..BBBB"
        ".BBBBB"
        "AAABBC");

    EXPECT_EQ(1, pf.height(1));
    EXPECT_EQ(2, pf.height(2));
    EXPECT_EQ(3, pf.height(3));
    EXPECT_EQ(4, pf.height(4));
    EXPECT_EQ(5, pf.height(5));
    EXPECT_EQ(6, pf.height(6));

    EXPECT_EQ('A', pf.variable(1, 1));
    EXPECT_EQ('B', pf.variable(2, 2));
    EXPECT_EQ('C', pf.variable(6, 1));
    EXPECT_EQ(' ', pf.variable(1, 2));
}

TEST(PatternFieldTest, constructor2)
{
    PatternField pf(vector<string> {
        ".....B",
        "....BB",
        "...BBB",
        "..BBBB",
        ".BBBBB",
        "AAABBC"
    });

    EXPECT_EQ(1, pf.height(1));
    EXPECT_EQ(2, pf.height(2));
    EXPECT_EQ(3, pf.height(3));
    EXPECT_EQ(4, pf.height(4));
    EXPECT_EQ(5, pf.height(5));
    EXPECT_EQ(6, pf.height(6));

    EXPECT_EQ('A', pf.variable(1, 1));
    EXPECT_EQ('B', pf.variable(2, 2));
    EXPECT_EQ('C', pf.variable(6, 1));
    EXPECT_EQ(' ', pf.variable(1, 2));
}

TEST(PatternFieldTest, varCount)
{
    PatternField pf1("AAA...");
    PatternField pf2("..AAAB");
    PatternField pf3(".*ABBB");

    EXPECT_EQ(3, pf1.numVariables());
    EXPECT_EQ(4, pf2.numVariables());
    EXPECT_EQ(4, pf3.numVariables()); // We don't count *

    {
        PatternField f;
        ASSERT_TRUE(PatternField::merge(pf1, pf2, &f));
        EXPECT_EQ(6, f.numVariables());
    }
    {
        PatternField f;
        ASSERT_TRUE(PatternField::merge(pf1, pf2, &f));
        EXPECT_EQ(6, f.numVariables());
    }
}

