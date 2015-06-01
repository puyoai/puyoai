#include "pattern_book_field.h"

#include <gtest/gtest.h>

TEST(PatternBookFieldTest, ignitionPositions)
{
    PatternBookField pbf(
        "A....."
        "ABC..."
        "AAB..."
        "BBCCC.", "name", 3, 1.0);

    FieldBits expected(
        "1....."
        "1....."
        "11...."
        "......");

    FieldBits expectedMirror(
        ".....1"
        ".....1"
        "....11"
        "......");

    EXPECT_EQ(expected, pbf.ignitionPositions());
    EXPECT_EQ(expectedMirror, pbf.mirror().ignitionPositions());
}
