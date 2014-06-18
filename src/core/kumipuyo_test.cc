#include "core/kumipuyo.h"

#include <gtest/gtest.h>

TEST(KumipuyoPosTest, BasicTest)
{
    KumipuyoPos r0(3, 4, 0);
    EXPECT_EQ(3, r0.axisX());
    EXPECT_EQ(4, r0.axisY());
    EXPECT_EQ(3, r0.childX());
    EXPECT_EQ(5, r0.childY());

    KumipuyoPos r1(3, 4, 1);
    EXPECT_EQ(3, r1.axisX());
    EXPECT_EQ(4, r1.axisY());
    EXPECT_EQ(4, r1.childX());
    EXPECT_EQ(4, r1.childY());

    KumipuyoPos r2(3, 4, 2);
    EXPECT_EQ(3, r2.axisX());
    EXPECT_EQ(4, r2.axisY());
    EXPECT_EQ(3, r2.childX());
    EXPECT_EQ(3, r2.childY());

    KumipuyoPos r3(3, 4, 3);
    EXPECT_EQ(3, r3.axisX());
    EXPECT_EQ(4, r3.axisY());
    EXPECT_EQ(2, r3.childX());
    EXPECT_EQ(4, r3.childY());
}
