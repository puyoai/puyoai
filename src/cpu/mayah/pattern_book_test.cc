#include "pattern_book.h"

#include <gtest/gtest.h>

using namespace std;

static const char TEST_BOOK[] = R"(
[[pattern]]
field = [
    "A.....",
    "ABC...",
    "AABCC.",
    "BBC@@.",
]
ignition = 1
)";

static const char TEST_BOOK2[] = R"(
[[pattern]]
field = [
    "A.....",
    "AB....",
    "AAC...",
    "BBBC..",
    "CCC@..",
]
ignition = 1

[[pattern]]
field = [
    ".....C",
    "...B.C",
    "..AABB",
    "AAABCC",
]
ignition = 2

)";

static const char TEST_BOOK3[] = R"(
[[pattern]]
field = [
    ".A....",
    "BA....",
    "AA....",
    "BC....",
    "BBC...",
    "CC....",
]
ignition = 2

[[pattern]]
field = [
    "A.....",
    "ABC...",
    "AABCC.",
    "BBC@@.",
]
ignition = 1
)";

TEST(PatternBookTest, pattern1)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(TEST_BOOK));

    CoreField field("G....."
                    "G.Y..."
                    "YYB...");

    CoreField expected("G....."
                       "GYB..."
                       "GGYBB."
                       "YYBOO.");

    bool found = false;
    auto callback = [&](const CoreField&,
                        const RensaResult&,
                        const ColumnPuyoList& keyPuyos,
                        const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&) {
        CoreField cf(field);
        for (const auto& cp : keyPuyos)
            cf.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            cf.dropPuyoOn(cp.x, cp.color);

        if (expected == cf)
            found = true;
    };

    patternBook.iteratePossibleRensas(field, 1, callback);
    EXPECT_TRUE(found);
}

TEST(PatternBookTest, pattern2)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(TEST_BOOK2));

    CoreField field("G....."
                    "R.RY.."
                    "YYYBYY");

    CoreField expected("G....."
                       "GR...Y"
                       "GGYB.Y"
                       "RRRYBB"
                       "YYYBYY");

    bool found = false;
    auto callback = [&](const CoreField&,
                        const RensaResult&,
                        const ColumnPuyoList& keyPuyos,
                        const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&) {
        CoreField cf(field);
        for (const auto& cp : keyPuyos)
            cf.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            cf.dropPuyoOn(cp.x, cp.color);

        if (expected == cf)
            found = true;
    };

    patternBook.iteratePossibleRensas(field, 2, callback);
    EXPECT_TRUE(found);
}

TEST(PatternBookTest, pattern3)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(TEST_BOOK3));

    CoreField field("G....."
                    "R....."
                    "RRY..."
                    "YYB...");

    CoreField expected(".G...."
                       "RG...."
                       "GG...."
                       "RYB..."
                       "RRYBB."
                       "YYB@@.");

    bool found = false;
    auto callback = [&](const CoreField&,
                        const RensaResult&,
                        const ColumnPuyoList& keyPuyos,
                        const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&) {
        CoreField cf(field);
        for (const auto& cp : keyPuyos)
            cf.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            cf.dropPuyoOn(cp.x, cp.color);

        if (expected == cf)
            found = true;
    };

    patternBook.iteratePossibleRensas(field, 2, callback);
    EXPECT_TRUE(found);
}
