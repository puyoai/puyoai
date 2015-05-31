#include "core/field_checker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(FieldCheckerTest, initialize)
{
    FieldChecker checker;

    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y) {
            EXPECT_FALSE(checker.get(x, y));
        }
    }
}

TEST(FieldCheckerTest, getAndSet)
{
    FieldChecker checker;

    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y) {
            checker.set(x, y);
            EXPECT_TRUE(checker.get(x, y));
            checker.clear(x, y);
            EXPECT_FALSE(checker.get(x, y));
        }
    }
}

TEST(FieldCheckerTest, setBit)
{
    FieldChecker checker;
    checker.setBit(1, 3, true);
    EXPECT_TRUE(checker.get(1, 3));
    checker.setBit(1, 3, false);
    EXPECT_FALSE(checker.get(1, 3));
}
