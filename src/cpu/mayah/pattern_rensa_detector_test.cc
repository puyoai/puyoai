#include "pattern_rensa_detector.h"

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
score = 72
name = "GTR"
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
name = "NEWGTR"

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

static const char TEST_BOOK4[] = R"(
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
    "......",
    "A.....",
    "ABC...",
    "AABCC.",
    "BBC...",
]
ignition = 1

[[pattern]]
field = [
    ".....C",
    "...BBC",
    "..AAAB",
    "..ACCB",
]
ignition = 3
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
                        const RensaTrackResult&,
                        const std::string& patternName,
                        int patternScore) {
        CoreField cf(field);
        cf.dropPuyoList(keyPuyos);
        cf.dropPuyoList(firePuyos);

        if (expected == cf) {
            found = true;
            EXPECT_EQ(72.0 * (6.0 / 12.0), patternScore);
            EXPECT_EQ("GTR", patternName);
        }
    };

    PatternRensaDetector(patternBook, field, callback).iteratePossibleRensas({0}, 1);
    EXPECT_TRUE(found);
}

TEST(PatternBookTest, pattern1_complement)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(TEST_BOOK));

    CoreField field("G....."
                    "G....."
                    "YYB...");

    CoreField expected1("G....."
                        "GYB..."
                        "GGYBB."
                        "YYBOO.");
    CoreField expected2("G....."
                        "GYR..."
                        "GGYRR."
                        "YYROO.");

    bool found = false;
    auto callback = [&](const CoreField&,
                        const RensaResult&,
                        const ColumnPuyoList& keyPuyos,
                        const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&,
                        const std::string& patternName,
                        int patternScore) {
        CoreField cf(field);
        cf.dropPuyoList(keyPuyos);
        cf.dropPuyoList(firePuyos);

        if (cf == expected1 || cf == expected2) {
            found = true;
            EXPECT_EQ(72.0 * (5.0 / 12.0), patternScore);
            EXPECT_EQ("GTR", patternName);
        }
    };

    PatternRensaDetector(patternBook, field, callback).iteratePossibleRensas({0}, 1);
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
                        const RensaTrackResult&,
                        const std::string& patternName,
                        int /*patternScore*/) {
        CoreField cf(field);
        cf.dropPuyoList(keyPuyos);
        cf.dropPuyoList(firePuyos);

        if (expected == cf) {
            found = true;
            EXPECT_EQ("NEWGTR", patternName);
        }
    };

    PatternRensaDetector(patternBook, field, callback).iteratePossibleRensas({0, 1}, 2);
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
                        const RensaTrackResult&,
                        const std::string&,
                        int /*patternScore*/) {
        CoreField cf(field);
        for (const auto& cp : keyPuyos)
            cf.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            cf.dropPuyoOn(cp.x, cp.color);

        if (expected == cf)
            found = true;
    };

    PatternRensaDetector(patternBook, field, callback).iteratePossibleRensas({0, 1}, 2);
    EXPECT_TRUE(found);
}

TEST(PatternBookTest, pattern4)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(TEST_BOOK4));

    CoreField field("G....."
                    "RBR..."
                    "RRBRR."
                    "BBRYY.");

    CoreField expected1(".G...."
                        "RG...."
                        "GG...Y"
                        "RBRBBY"
                        "RRBRRB"
                        "BBRYYB");

    CoreField expected2(".G...."
                        "RG...."
                        "GG...Y"
                        "RBRGGY"
                        "RRBRRG"
                        "BBRYYG");

    bool found = false;
    auto callback = [&](const CoreField&,
                        const RensaResult&,
                        const ColumnPuyoList& keyPuyos,
                        const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&,
                        const std::string&,
                        int /*patternScore*/) {
        CoreField cf(field);
        ASSERT_TRUE(cf.dropPuyoList(keyPuyos));
        ASSERT_TRUE(cf.dropPuyoList(firePuyos));

        if (cf == expected1 || cf == expected2)
            found = true;
    };

    PatternRensaDetector(patternBook, field, callback).iteratePossibleRensas({0, 1, 2}, 3);
    EXPECT_TRUE(found);
}
