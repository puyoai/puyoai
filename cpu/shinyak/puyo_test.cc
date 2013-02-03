#include "puyo.h"

#include <gtest/gtest.h>

TEST(PuyoTest, IsColorPuyoTest)
{
    EXPECT_FALSE(isColorPuyo(EMPTY));
    EXPECT_FALSE(isColorPuyo(OJAMA));
    EXPECT_FALSE(isColorPuyo(WALL));

    EXPECT_TRUE(isColorPuyo(RED));
    EXPECT_TRUE(isColorPuyo(BLUE));
    EXPECT_TRUE(isColorPuyo(YELLOW));
    EXPECT_TRUE(isColorPuyo(GREEN));
}
