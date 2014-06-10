#include "core/puyo_color.h"

#include <gtest/gtest.h>

TEST(PuyoTest, IsColorPuyoTest)
{
    EXPECT_FALSE(isNormalColor(PuyoColor::EMPTY));
    EXPECT_FALSE(isNormalColor(PuyoColor::OJAMA));
    EXPECT_FALSE(isNormalColor(PuyoColor::WALL));

    EXPECT_TRUE(isNormalColor(PuyoColor::RED));
    EXPECT_TRUE(isNormalColor(PuyoColor::BLUE));
    EXPECT_TRUE(isNormalColor(PuyoColor::YELLOW));
    EXPECT_TRUE(isNormalColor(PuyoColor::GREEN));
}
