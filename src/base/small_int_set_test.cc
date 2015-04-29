#include "base/small_int_set.h"

#include <gtest/gtest.h>

TEST(SmallIntSetTest, basic)
{
    SmallIntSet s;
    EXPECT_TRUE(s.isEmpty());
    EXPECT_EQ(0, s.size());

    s.set(0);
    s.set(1);
    s.set(5);
    s.set(8);
    s.set(16);
    s.set(31);
    s.set(31);

    EXPECT_EQ(6, s.size());

    s.unset(8);
    EXPECT_EQ(5, s.size());

    EXPECT_EQ(0, s.smallest());
    s.removeSmallest();

    EXPECT_EQ(1, s.smallest());
    s.removeSmallest();

    EXPECT_EQ(5, s.smallest());
    s.removeSmallest();

    EXPECT_EQ(16, s.smallest());
    s.removeSmallest();

    EXPECT_EQ(31, s.smallest());
    s.removeSmallest();

    EXPECT_TRUE(s.isEmpty());
    EXPECT_EQ(0, s.size());
}
