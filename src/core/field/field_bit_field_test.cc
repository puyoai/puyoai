#include "core/field/field_bit_field.h"

#include <gtest/gtest.h>
#include "core/field/core_field.h"

using namespace std;

TEST(FieldBitFieldTest, initialize)
{
    FieldBitField bitField;

    for (int x = 0; x < CoreField::MAP_WIDTH; ++x) {
        for (int y = 0; y < CoreField::MAP_HEIGHT; ++y) {
            EXPECT_EQ(bitField.get(x, y), 0);
        }
    }
}

TEST(FieldBitFieldTest, getAndSet)
{
    FieldBitField bitField;

    for (int x = 0; x < CoreField::MAP_WIDTH; ++x) {
        for (int y = 0; y < CoreField::MAP_HEIGHT; ++y) {
            bitField.set(x, y);
            EXPECT_EQ(bitField.get(x, y), 1);
            bitField.clear(x, y);
            EXPECT_EQ(bitField.get(x, y), 0);
        }
    }
}
