#include "core/probability/puyo_set.h"

#include <gtest/gtest.h>

#include "core/column_puyo_list.h"

TEST(PuyoSetTest, ctor)
{
    PuyoSet set1;
    EXPECT_EQ(0, set1.red());
    EXPECT_EQ(0, set1.blue());
    EXPECT_EQ(0, set1.yellow());
    EXPECT_EQ(0, set1.green());

    PuyoSet set2(1, 2, 3, 4);
    EXPECT_EQ(1, set2.red());
    EXPECT_EQ(2, set2.blue());
    EXPECT_EQ(3, set2.yellow());
    EXPECT_EQ(4, set2.green());
}

TEST(PuyoSetTest, count)
{
    {
        PuyoSet ps;
        EXPECT_EQ(0, ps.count());
    }
    {
        PuyoSet ps(1, 2, 3, 4);
        EXPECT_EQ(10, ps.count());
    }
}

TEST(PuyoSetTest, addPuyoColor)
{
    PuyoSet set;
    set.add(PuyoColor::RED, 1);
    set.add(PuyoColor::BLUE, 2);
    set.add(PuyoColor::YELLOW, 3);
    set.add(PuyoColor::GREEN, 4);

    EXPECT_EQ(1, set.red());
    EXPECT_EQ(2, set.blue());
    EXPECT_EQ(3, set.yellow());
    EXPECT_EQ(4, set.green());
}

TEST(PuyoSetTest, addPuyoSet)
{
    PuyoSet set1, set2;
    set1.add(PuyoColor::RED, 1);
    set1.add(PuyoColor::GREEN, 1);
    set2.add(PuyoColor::RED, 1);
    set2.add(PuyoColor::BLUE, 1);

    PuyoSet set;
    set.add(set1);
    set.add(set2);
    EXPECT_EQ(2, set.red());
    EXPECT_EQ(1, set.blue());
    EXPECT_EQ(0, set.yellow());
    EXPECT_EQ(1, set.green());
}

TEST(PuyoSetTest, addPuyoSetLarge)
{
    PuyoSet set1(123456, 123456, 123456, 123456);
    PuyoSet set2(1, 2, 3, 4);

    PuyoSet set;
    set.add(set1);
    set.add(set2);

    EXPECT_EQ(123457, set.red());
    EXPECT_EQ(123458, set.blue());
    EXPECT_EQ(123459, set.yellow());
    EXPECT_EQ(123460, set.green());
}

TEST(PuyoSetTest, subPuyoSet)
{
    PuyoSet set1(1, 2, 3, 4);
    PuyoSet set2(2, 2, 2, 2);

    set1.sub(set2);

    EXPECT_EQ(0, set1.red());
    EXPECT_EQ(0, set1.blue());
    EXPECT_EQ(1, set1.yellow());
    EXPECT_EQ(2, set1.green());
}

TEST(PuyoSetTest, addColumnPuyoList)
{
    ColumnPuyoList cpl;
    cpl.add(1, PuyoColor::RED);
    cpl.add(1, PuyoColor::YELLOW);
    cpl.add(2, PuyoColor::RED);
    cpl.add(3, PuyoColor::RED);

    PuyoSet set;
    set.add(cpl);

    EXPECT_EQ(3, set.red());
    EXPECT_EQ(0, set.blue());
    EXPECT_EQ(1, set.yellow());
    EXPECT_EQ(0, set.green());
}
