#include "pattern_book.h"

#include <gtest/gtest.h>
#include <iostream>

#include "base/base.h"

using namespace std;

namespace {

void testMatch(const char* book, const CoreField& original)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(book));

    bool found = false;
    auto callback = [&](CoreField&& /*cf*/, const ColumnPuyoList& /*cpl*/,
                        int /*numFilledUnusedVariables*/, const FieldBits& /*matchedBits*/,
                        const PatternBookField& /*patternBookField*/) {
        found = true;
    };

    patternBook.complement(original, callback);
    EXPECT_TRUE(found);
}

void testUnmatch(const char* book, const CoreField& original)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(book));

    bool found = false;
    CoreField foundField;
    auto callback = [&](CoreField&& cf, const ColumnPuyoList& /*cpl*/,
                        int /*numFilledUnusedVariables*/, const FieldBits& /*matchedBits*/,
                        const PatternBookField& /*patternBookField*/) {
        found = true;
        foundField = cf;
    };

    patternBook.complement(original, callback);
    EXPECT_FALSE(found) << foundField;
}

void testComplement(const char* book, const CoreField& original,
                    const CoreField expected[], size_t size,
                    int numAllowedUnusedVariables = 0,
                    bool allowsUnexpected = false)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(book));

    vector<bool> found(size);
    auto callback = [&](CoreField&& cf, const ColumnPuyoList& /*cpl*/,
                        int /*numFilledUnusedVariables*/, const FieldBits& /*matchedBits*/,
                        const PatternBookField& /*patternBookField*/) {
        bool expectedFound = false;
        for (size_t i = 0; i < size; ++i) {
            if (expected[i] == cf) {
                found[i] = true;
                expectedFound = true;
            }
        }

        if (!allowsUnexpected) {
            EXPECT_TRUE(expectedFound) << cf.toDebugString();
        }
    };
    patternBook.complement(original, numAllowedUnusedVariables, callback);

    for (size_t i = 0; i < size; ++i)
        EXPECT_TRUE(found[i]) << i;
}

void testComplementWithIgnitionBits(const char* book,
                                    const CoreField& original,
                                    const FieldBits& ignitionBits,
                                    const CoreField expected[], size_t size,
                                    bool allowsUnexpected = false)
{
    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(book));

    vector<bool> found(size);
    auto callback = [&](CoreField&& cf, const ColumnPuyoList& /*cpl*/,
                        int /*numFilledUnusedVariables*/, const FieldBits& /*matchedBits*/,
                        const PatternBookField& /*patternBookField*/) {
        bool expectedFound = false;
        for (size_t i = 0; i < size; ++i) {
            if (expected[i] == cf) {
                found[i] = true;
                expectedFound = true;
            }
        }

        if (!allowsUnexpected) {
            EXPECT_TRUE(expectedFound) << cf.toDebugString();
        }
    };
    patternBook.complement(original, ignitionBits, 0, callback);

    for (size_t i = 0; i < size; ++i)
        EXPECT_TRUE(found[i]) << i;
}

} // namespace anonymous

TEST(PatternBookTest, complement)
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
    "A.....",
    "ABC...",
    "AABCCC",
    "BBC&&&",
]
ignition = 1
score = 72
name = "GTR"
)";

    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(BOOK));

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
                        const PatternBookField& patternBookField) {
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

TEST(PatternBookTest, complement1)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "A.BC..",
    "AAAB..",
    "BBBCCC",
]
ignition = 1
)";

    const CoreField original1(
        "......"
        "..YB.."
        "BBBG..");
    const CoreField original2(
        "..BG.."
        "YYYB.."
        "BBBGGG");

    const CoreField expected[] {
        CoreField(
            "Y.BG.."
            "YYYB.."
            "BBBGGG")
    };

    testComplement(BOOK, original1, expected, ARRAY_SIZE(expected));
    testComplement(BOOK, original2, expected, ARRAY_SIZE(expected));
}

TEST(PatternBookTest, complement2)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "A...De",
    "ABCDDE",
    "AABCCD",
    "BBCEEE",
]
ignition = 1
)";

    const CoreField original(
        ".....Y"
        "Y....Y"
        "Y..RRB"
        "GG.YYY");
    const CoreField expected[] {
        CoreField(
            "Y...BY"
            "YGRBBY"
            "YYGRRB"
            "GGRYYY")
    };

    testComplement(BOOK, original, expected, ARRAY_SIZE(expected));
}

TEST(PatternBookTest, complement3)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "AAA...",
])";

    const CoreField expected[] {
        CoreField("RRR...")
    };

    testUnmatch(BOOK, CoreField());
    testComplement(BOOK, CoreField("RRR..."), expected, ARRAY_SIZE(expected));
    testComplement(BOOK, CoreField("R....."), expected, ARRAY_SIZE(expected));
    testComplement(BOOK, CoreField("R.R..."), expected, ARRAY_SIZE(expected));
}

TEST(PatternBookTest, complementWithStar)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    ".***CC",
    ".AABBB",
])";

    CoreField f1(
        ".YYYGG"
        ".RRBBB");
    CoreField expected1[] {
        CoreField(
            ".YYYGG"
            ".RRBBB"),
    };

    CoreField f2(
        ".B...."
        ".BBYYY"
        ".RRBBB");
    CoreField expected2[] {
        CoreField(
            ".B...."
            ".BBYYY"
            ".RRBBB"),
    };

    CoreField f3(
        ".GGGYY"
        ".RRBBB");
    CoreField expected3[] {
        CoreField(
            ".GGGYY"
            ".RRBBB"),
    };

    testUnmatch(BOOK, CoreField());
    testComplement(BOOK, f1, expected1, ARRAY_SIZE(expected1));
    testComplement(BOOK, f2, expected2, ARRAY_SIZE(expected2));
    testComplement(BOOK, f3, expected3, ARRAY_SIZE(expected3));
}

TEST(PatternBookTest, complementWithAllow1)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    ".ab...",
    ".AB...",
    "ABC...",
    "ABC...",
    "ABCC..",
]
ignition = 4
)";

    const CoreField original(
        "YGB..."
        "YGB..."
        "YGB...");

    const CoreField expected[] {
        CoreField(
            ".YG..."
            "YGB..."
            "YGB..."
            "YGBB.."),
    };

    testComplement(BOOK, original, expected, ARRAY_SIZE(expected));
}

TEST(PatternBookTest, complementWithAllow2)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "AA.BB.",
    "AAaBB.",
]
)";

    CoreField original(
        "RRRBB.");

    CoreField expected[] {
        CoreField(
            "RR.BB."
            "RRRBB.")
    };

    testComplement(BOOK, original, expected, ARRAY_SIZE(expected));
}

TEST(PatternBookTest, complementWithUnusedVariable1)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "A.....",
    "ABC...",
    "AABCC.",
    "BBC&&.",
])";

    const CoreField original(
        "Y....."
        "Y....."
        "GG....");

    // TODO(mayah): C is OK for either R, B, or Y.
    const CoreField expected[] {
        CoreField(
            "Y....."
            "YGR..."
            "YYGRR."
            "GGR&&.")
    };

    testComplement(BOOK, original, expected, ARRAY_SIZE(expected), 1);
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

    PatternBook patternBook;
    ASSERT_TRUE(patternBook.loadFromString(BOOK));

    CoreField original(
        "R....."
        "R....."
        "RRB..."
        "BBYBB.");
    FieldBits ignitionBits = original.bitField().bits(PuyoColor::RED);

    CoreField expected[] {
        CoreField(
            "R....."
            "RBY..."
            "RRBYY."
            "BBYBB."),
    };

    testComplementWithIgnitionBits(BOOK, original, ignitionBits, expected, ARRAY_SIZE(expected));
}

TEST(PatternBookTest, unmatch1)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "A.....",
    "ABC.D.",
    "AABCCC",
    "BBDDD.",
]
ignition = 1
score = 1.0
)";

    CoreField f1(
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    CoreField f2(
        "....R."
        "BYB.R."
        "BBYBBB"
        "YYRRRG");

    CoreField f3(
        "BYO.R."
        "BBYOOO"
        "YYRRRY");

    CoreField f4(
        "BRB.R."
        "BBRBBB"
        "RRRRRY");

    testMatch(BOOK, f1);
    testUnmatch(BOOK, f2);
    testUnmatch(BOOK, f3);
    testUnmatch(BOOK, f4);
}

TEST(PatternBookTest, unmatch2)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    ".AAAAA",
]
ignition = 6
)";

    CoreField original("B..B.B");
    testUnmatch(BOOK, original);
}

TEST(PatternBookTest, unmatch3)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "A....B",
    "AAABBB",
]
ignition = 6
)";

    CoreField original("Y....Y");

    testUnmatch(BOOK, original);
}

TEST(PatternBookTest, unmatch4)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "BA....",
    "AAA...",
    "BC....",
    "BBC...",
    "CC....",
]
ignition = 2
)";

    CoreField original(
        "YB...."
        "YYB..."
        "BBGGG.");

    // Since we cannot complement (3, 3), this pattern should not match.
    testUnmatch(BOOK, original);
}

TEST(PatternBookTest, unmatch5)
{
    static const char BOOK[] = R"(
[[pattern]]
field = [
    "BA....",
    "AAA...",
    "BC....",
    "BBC...",
    "CC....",
]
ignition = 2
)";

    CoreField original(
        ".R...."
        "YY.BBB"
        "RRRGGG");

    testUnmatch(BOOK, original);
}
