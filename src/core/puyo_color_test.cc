#include "core/puyo_color.h"

#include <gtest/gtest.h>

TEST(PuyoTest, IsColorPuyoTest)
{
    EXPECT_FALSE(isColorPuyo(PuyoColor::EMPTY));
    EXPECT_FALSE(isColorPuyo(PuyoColor::OJAMA));
    EXPECT_FALSE(isColorPuyo(PuyoColor::WALL));

    EXPECT_TRUE(isColorPuyo(PuyoColor::RED));
    EXPECT_TRUE(isColorPuyo(PuyoColor::BLUE));
    EXPECT_TRUE(isColorPuyo(PuyoColor::YELLOW));
    EXPECT_TRUE(isColorPuyo(PuyoColor::GREEN));
}
