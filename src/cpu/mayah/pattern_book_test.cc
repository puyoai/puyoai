#include "pattern_book.h"

#include <gtest/gtest.h>
#include <iostream>

#include "base/base.h"

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

TEST(PatternBookTest, complement)
{
    NewPatternBook patternBook;
    patternBook.loadFromString(TEST_BOOK);

    CoreField original(
        "......"
        "RRB..."
        "BBYBB.");

    FieldBits expectedMatchedBits(
        "111..."
        "111...");

    CoreField expected[] {
        CoreField(
            "R....."
            "RBY..."
            "RRBYY."
            "BBYBB."),
        CoreField(
            "R....."
            "RBY..."
            "RRBYYY"
            "BBYBB&"),
    };
    ColumnPuyoList expectedColumnPuyoList[ARRAY_SIZE(expected)] {};
    expectedColumnPuyoList[0].add(1, PuyoColor::RED);
    expectedColumnPuyoList[0].add(1, PuyoColor::RED);
    expectedColumnPuyoList[0].add(2, PuyoColor::BLUE);
    expectedColumnPuyoList[0].add(3, PuyoColor::YELLOW);
    expectedColumnPuyoList[0].add(4, PuyoColor::YELLOW);
    expectedColumnPuyoList[0].add(5, PuyoColor::YELLOW);

    expectedColumnPuyoList[1].add(1, PuyoColor::RED);
    expectedColumnPuyoList[1].add(1, PuyoColor::RED);
    expectedColumnPuyoList[1].add(2, PuyoColor::BLUE);
    expectedColumnPuyoList[1].add(3, PuyoColor::YELLOW);
    expectedColumnPuyoList[1].add(4, PuyoColor::YELLOW);
    expectedColumnPuyoList[1].add(5, PuyoColor::YELLOW);
    expectedColumnPuyoList[1].add(6, PuyoColor::IRON);
    expectedColumnPuyoList[1].add(6, PuyoColor::YELLOW);

    bool found[ARRAY_SIZE(expected)] {};

    auto callback = [&](CoreField&& cf, const ColumnPuyoList& cpl,
                        int numFilledUnusedVariables, const FieldBits& matchedBits,
                        const NewPatternBookField& patternBookField) {
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (expected[i] != cf)
                continue;

            EXPECT_FALSE(found[i]) << "Duplicated " << i;
            found[i] = true;

            EXPECT_EQ(expectedColumnPuyoList[i], cpl);
            EXPECT_EQ(0, numFilledUnusedVariables);
            EXPECT_EQ(expectedMatchedBits, matchedBits);
            EXPECT_EQ(72, patternBookField.score());
        }
    };
    patternBook.complement(original, callback);
    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i)
        EXPECT_TRUE(found[i]) << i;
}

TEST(PatternBookTest, complementWithIgnitionBits)
{
    static const char BOOK[] = R"(
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
    "AA....",
    "ABC...",
    "AABCC.",
    "BBC&&.",
]
ignition = 1
score = 72
name = "GTR"
    )";

    NewPatternBook patternBook;
    patternBook.loadFromString(BOOK);

    CoreField original(
        "R....."
        "R....."
        "RRB..."
        "BBYBB.");

    CoreField expected(
        "R....."
        "RBY..."
        "RRBYY."
        "BBYBB.");

    bool found = false;
    bool foundUnexpected = false;

    auto callback = [&](CoreField&& cf, const ColumnPuyoList& /*cpl*/,
                        int /*numFilledUnusedVariables*/, const FieldBits& /*matchedBits*/,
                        const NewPatternBookField& /*patternBookField*/) {

        if (expected != cf) {
            foundUnexpected = true;
            return;
        }

        found = true;
    };

    patternBook.complement(original, original.bitField().bits(PuyoColor::RED), 0, callback);

    EXPECT_TRUE(found);
    EXPECT_FALSE(foundUnexpected);
}
