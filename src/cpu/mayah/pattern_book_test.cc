#include "pattern_book.h"

#include <gtest/gtest.h>
#include <iostream>

using namespace std;

static const char TEST_BOOK[] = R"(
[[pattern]]
field = [
    "A.....",
    "ABC...",
    "AABCC.",
    "BBC&&.",
]
ignition = 1
score = 72
name = "GTR"

[[pattern]]
field = [
    "A.....",
    "ABC...",
    "AABCCC",
    "BBC&&&",
]
ignition = 1
score = 72
name = "GTR"
)";

TEST(PatternBookTest, ignitionPositions)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(TEST_BOOK));

    FieldBits bits(
        ".....1"
        ".....1"
        "....11"
        "......");

    auto itPair = patternBook.find(bits);

    int count = 0;
    for (auto it = itPair.first; it != itPair.second; ++it) {
        ++count;
    }

    EXPECT_EQ(2, count);
}
