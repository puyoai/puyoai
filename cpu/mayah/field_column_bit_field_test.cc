#include "field_column_bit_field.h"

#include <gtest/gtest.h>

TEST(FieldColumnBitFieldTest, IndexFor34Test1)
{
    Field f("111111"
            "456444"
            "456777"
            "456444");

    FieldColumnBitField bitField(f);
    EXPECT_EQ(0xA07 | (0x38 << 12), bitField.indexFor34(1, 1, RED, BLUE));
}

TEST(FieldColumnBitFieldTest, IndexFor34Test2)
{
    Field f("111111"
            "145644"
            "145677"
            "145644");

    FieldColumnBitField bitField(f);
    EXPECT_EQ(0xA07 | (0x38 << 12), bitField.indexFor34(2, 1, RED, BLUE));
}

TEST(FieldColumnBitFieldTest, IndexFor34Test3)
{
    Field f("111111"
            "145644"
            "145677"
            "145644");

    FieldColumnBitField bitField(f);
    // 0b0000_0010_1101   0b1110_0000_0000
    EXPECT_EQ(0x2D | (0xE00 << 12), bitField.indexFor34(3, 1, RED, BLUE));
}

TEST(FieldColumnBitFieldTest, IndexFor43Test1)
{
    Field f("111111"
            "456444"
            "455777"
            "454444");

    FieldColumnBitField bitField(f);
    // 0b0001_0000_0111 0b0010_0111_0000
    EXPECT_EQ(0x107 | (0x270 << 12), bitField.indexFor43(1, 1, RED, BLUE));
}

TEST(FieldColumnBitFieldTest, IndexFor43Test2)
{
    Field f("111445"
            "456444"
            "456555"
            "456444");

    FieldColumnBitField bitField(f);
    // 0b1101_1101_0101   0b0010_0010_1010
    EXPECT_EQ(0xDD5 | (0x22A << 12), bitField.indexFor43(4, 1, RED, BLUE));
}
