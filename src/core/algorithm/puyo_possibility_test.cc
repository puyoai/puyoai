#include "puyo_possibility.h"

#include <gtest/gtest.h>

TEST(TsumoPossibilityTest, Possibility)
{
    TsumoPossibility::initialize();

    EXPECT_EQ(TsumoPossibility::possibility(0, 0, 0, 0, 0), 1.0);
    EXPECT_EQ(TsumoPossibility::possibility(1, 0, 0, 0, 0), 1.0);
    EXPECT_EQ(TsumoPossibility::possibility(1, 1, 0, 0, 0), 1.0 / 4.0);
    EXPECT_EQ(TsumoPossibility::possibility(2, 1, 1, 0, 0), 1.0 / 8.0);
    EXPECT_EQ(TsumoPossibility::possibility(2, 2, 0, 0, 0), 1.0 / 16.0);
}
