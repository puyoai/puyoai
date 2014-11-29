#include "pattern_field.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

using namespace std;

TEST(PatternFieldTest, initial)
{
    PatternField pf;
    for (int i = 1; i <= 6; ++i) {
        EXPECT_EQ(0, pf.height(i));
    }
}

TEST(PatternFieldTest, constructor)
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

    EXPECT_EQ('A', pf.get(1, 1));
    EXPECT_EQ('B', pf.get(2, 2));
    EXPECT_EQ('C', pf.get(6, 1));
    EXPECT_EQ(' ', pf.get(1, 2));
}
