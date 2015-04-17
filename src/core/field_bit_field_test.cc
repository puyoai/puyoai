#include "core/field_bit_field.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(FieldBitFieldTest, initialize)
{
    FieldBitField bitField;

    for (int x = 0; x < CoreField::MAP_WIDTH; ++x) {
        for (int y = 0; y < CoreField::MAP_HEIGHT; ++y) {
            EXPECT_FALSE(bitField.get(x, y));
            EXPECT_FALSE(bitField(x, y));
        }
    }
}

TEST(FieldBitFieldTest, getAndSet)
{
    FieldBitField bitField;

    for (int x = 0; x < CoreField::MAP_WIDTH; ++x) {
        for (int y = 0; y < CoreField::MAP_HEIGHT; ++y) {
            bitField.set(x, y);
            EXPECT_TRUE(bitField(x, y));
            bitField.clear(x, y);
            EXPECT_FALSE(bitField(x, y));
        }
    }
}
