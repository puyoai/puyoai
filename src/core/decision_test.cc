#include "core/decision.h"

#include <gtest/gtest.h>

using namespace std;

TEST(DecisionTest, xr)
{
    Decision d10(3, 0);
    Decision d11(3, 1);
    Decision d12(3, 2);
    Decision d13(3, 3);

    EXPECT_EQ(3, d10.axisX());
    EXPECT_EQ(3, d10.childX());
    EXPECT_EQ(0, d10.rot());

    EXPECT_EQ(3, d11.axisX());
    EXPECT_EQ(4, d11.childX());
    EXPECT_EQ(1, d11.rot());

    EXPECT_EQ(3, d12.axisX());
    EXPECT_EQ(3, d12.childX());
    EXPECT_EQ(2, d12.rot());

    EXPECT_EQ(3, d13.axisX());
    EXPECT_EQ(2, d13.childX());
    EXPECT_EQ(3, d13.rot());
}

TEST(DecisionTest, valid)
{
    EXPECT_TRUE(Decision(1, 0).isValid());
    EXPECT_TRUE(Decision(1, 1).isValid());
    EXPECT_TRUE(Decision(1, 2).isValid());
    EXPECT_FALSE(Decision(1, 3).isValid());
    EXPECT_FALSE(Decision(1, 4).isValid());
    EXPECT_FALSE(Decision(1, -1).isValid());

    EXPECT_TRUE(Decision(2, 0).isValid());
    EXPECT_TRUE(Decision(2, 1).isValid());
    EXPECT_TRUE(Decision(2, 2).isValid());
    EXPECT_TRUE(Decision(2, 3).isValid());
    EXPECT_FALSE(Decision(2, 4).isValid());
    EXPECT_FALSE(Decision(2, -1).isValid());

    EXPECT_TRUE(Decision(6, 0).isValid());
    EXPECT_FALSE(Decision(6, 1).isValid());
    EXPECT_TRUE(Decision(6, 2).isValid());
    EXPECT_TRUE(Decision(6, 3).isValid());
    EXPECT_FALSE(Decision(6, 4).isValid());
    EXPECT_FALSE(Decision(6, -1).isValid());
}
