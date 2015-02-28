#include "pattern_book.h"

#include <gtest/gtest.h>

static const char TEST_BOOK[] = R"(
[[pattern]]
field = [
    "A.....",
    "ABC...",
    "AABCC.",
    "BBC...",
]
ignition = 1
)";

TEST(PatternBookTest, pattern)
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
