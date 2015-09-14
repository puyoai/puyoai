#include "core/column_puyo_list.h"

#include <gtest/gtest.h>

TEST(ColumnPuyoListTest, constructor)
{
    ColumnPuyoList cpl;
    EXPECT_EQ(0, cpl.size());
    EXPECT_TRUE(cpl.isEmpty());
}

TEST(ColumnPuyoListTest, copy)
{
    ColumnPuyoList cpl;
    cpl.add(1, PuyoColor::RED);
    cpl.add(1, PuyoColor::BLUE);
    cpl.add(3, PuyoColor::RED);
    cpl.add(3, PuyoColor::RED);

    ColumnPuyoList copied(cpl);

    for (int x = 1; x <= 6; ++x) {
        EXPECT_EQ(cpl.sizeOn(x), copied.sizeOn(x));
        for (int i = 0; i < cpl.sizeOn(x); ++i) {
            EXPECT_EQ(cpl.get(x, i), copied.get(x, i));
        }
    }
}

TEST(ColumnPuyoListTest, add)
{
    ColumnPuyoList cpl;
    ASSERT_TRUE(cpl.add(3, PuyoColor::RED, 2));

    EXPECT_EQ(2, cpl.size());
    EXPECT_FALSE(cpl.isEmpty());
}

TEST(ColumnPuyoListTest, merge)
{
    ColumnPuyoList cpl;
    ColumnPuyoList cpl1;
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED, 2));
    ColumnPuyoList cpl2;
    ASSERT_TRUE(cpl2.add(3, PuyoColor::BLUE, 2));
    ColumnPuyoList cpl3;
    ASSERT_TRUE(cpl3.add(3, PuyoColor::BLUE, 8));

    EXPECT_TRUE(cpl.merge(cpl1));
    EXPECT_TRUE(cpl.merge(cpl2));
    EXPECT_EQ(4, cpl.size());

    EXPECT_FALSE(cpl.merge(cpl3));
    EXPECT_EQ(4, cpl.size());
}

TEST(ColumnPuyoListTest, mergeWithPlaceHolders1)
{
    ColumnPuyoList cpl1;
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));

    ColumnPuyoList cpl2;
    ASSERT_TRUE(cpl2.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl2.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));

    EXPECT_TRUE(cpl1.merge(cpl2));

    EXPECT_EQ(7, cpl1.size());
    EXPECT_EQ(7, cpl1.sizeOn(3));
    EXPECT_EQ(PuyoColor::IRON, cpl1.get(3, 0));
    EXPECT_EQ(PuyoColor::IRON, cpl1.get(3, 1));
    EXPECT_EQ(PuyoColor::YELLOW, cpl1.get(3, 2));
    EXPECT_EQ(PuyoColor::RED, cpl1.get(3, 3));
    EXPECT_EQ(PuyoColor::RED, cpl1.get(3, 4));
    EXPECT_EQ(PuyoColor::RED, cpl1.get(3, 5));
    EXPECT_EQ(PuyoColor::YELLOW, cpl1.get(3, 6));
}

TEST(ColumnPuyoListTest, mergeWithPlaceHolders2)
{
    ColumnPuyoList cpl1;
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));

    ColumnPuyoList cpl2;
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));

    EXPECT_TRUE(cpl1.merge(cpl2));

    EXPECT_EQ(6, cpl1.size());
    EXPECT_EQ(6, cpl1.sizeOn(3));
    EXPECT_EQ(PuyoColor::IRON, cpl1.get(3, 0));
    EXPECT_EQ(PuyoColor::YELLOW, cpl1.get(3, 1));
    EXPECT_EQ(PuyoColor::YELLOW, cpl1.get(3, 2));
    EXPECT_EQ(PuyoColor::RED, cpl1.get(3, 3));
    EXPECT_EQ(PuyoColor::RED, cpl1.get(3, 4));
    EXPECT_EQ(PuyoColor::RED, cpl1.get(3, 5));
}

TEST(ColumnPuyoListTest, mergeWithPlaceHolders3)
{
    ColumnPuyoList cpl1;
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::IRON));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED));

    ColumnPuyoList cpl2;
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));
    ASSERT_TRUE(cpl2.add(3, PuyoColor::YELLOW));

    EXPECT_FALSE(cpl1.merge(cpl2));
}
