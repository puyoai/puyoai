#include <string>

#include <gtest/gtest.h>

#include <core/field.h>

#include "field_util.h"

TEST(FieldUtilTest, DropFlyingPuyos) {
  Field f("456756"
          "000000"
          "000000"
          "654765"
          "000000");
  DropFlyingPuyos(&f);
  EXPECT_EQ(6, f.Get(1, 1));
  EXPECT_EQ(5, f.Get(2, 1));
  EXPECT_EQ(4, f.Get(3, 1));
  EXPECT_EQ(7, f.Get(4, 1));
  EXPECT_EQ(6, f.Get(5, 1));
  EXPECT_EQ(5, f.Get(6, 1));
  EXPECT_EQ(4, f.Get(1, 2));
  EXPECT_EQ(5, f.Get(2, 2));
  EXPECT_EQ(6, f.Get(3, 2));
  EXPECT_EQ(7, f.Get(4, 2));
  EXPECT_EQ(5, f.Get(5, 2));
  EXPECT_EQ(6, f.Get(6, 2));
  EXPECT_EQ(0, f.Get(1, 3));
  EXPECT_EQ(0, f.Get(2, 3));
  EXPECT_EQ(0, f.Get(3, 3));
  EXPECT_EQ(0, f.Get(4, 3));
  EXPECT_EQ(0, f.Get(5, 3));
  EXPECT_EQ(0, f.Get(6, 3));
}
