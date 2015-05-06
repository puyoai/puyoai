#include "core/field_checker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(FieldCheckerTest, initialize)
{
    FieldChecker bitField;

    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y) {
            EXPECT_FALSE(bitField.get(x, y));
            EXPECT_FALSE(bitField(x, y));
        }
    }
}

TEST(FieldCheckerTest, getAndSet)
{
    FieldChecker bitField;

    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y) {
            bitField.set(x, y);
            EXPECT_TRUE(bitField(x, y));
            bitField.clear(x, y);
            EXPECT_FALSE(bitField(x, y));
        }
    }
}
