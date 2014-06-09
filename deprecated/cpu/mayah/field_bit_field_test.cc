#include "field_bit_field.h"

#include <gtest/gtest.h>
#include "field.h"

using namespace std;

TEST(FieldBitFieldTest, Initialize)
{
    FieldBitField bitField;

    for (int x = 0; x < Field::MAP_WIDTH; ++x) {
        for (int y = 0; y < Field::MAP_HEIGHT; ++y) {
            EXPECT_EQ(bitField.get(x, y), 0);
        }
    }
}

TEST(FieldBitFieldTest, GetAndSet)
{
    FieldBitField bitField;

    for (int x = 0; x < Field::MAP_WIDTH; ++x) {
        for (int y = 0; y < Field::MAP_HEIGHT; ++y) {
            bitField.set(x, y);
            EXPECT_EQ(bitField.get(x, y), 1);
            bitField.clear(x, y);
            EXPECT_EQ(bitField.get(x, y), 0);
        }
    }    
}
