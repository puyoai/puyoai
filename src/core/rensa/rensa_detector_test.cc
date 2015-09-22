#include "core/rensa/rensa_detector.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <utility>

#include "base/base.h"
#include "core/column_puyo.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/puyo_color.h"
#include "core/rensa_result.h"

using namespace std;

TEST(RensaDetectorTest, detectByDropStrategy1)
{
    const CoreField original(
        ".RGYG."
        "RGYGB."
        "RGYGB."
        "RGYGB.");

    const CoreField expected(
        ".RGYG."
        "RGYGB."
        "RGYGB."
        "RGYGBB");

    bool found = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        if (actual != expected)
            return;
        EXPECT_FALSE(found);
        found = true;

        CoreField cf(original);
        EXPECT_TRUE(cf.dropPuyoList(cpl));
        EXPECT_EQ(expected, cf);
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByDropStrategy(original, noProhibits, PurposeForFindingRensa::FOR_FIRE, 1, 12, callback);

    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, detectByFloatStrategy1)
{
    const CoreField original(
        "Y     "
        "B     "
        "R     "
        "B     "
        "B     "
        "B     "
        "Y     "
        "Y     "
        "Y     ");

    const CoreField expected(
        "Y     "
        "B     "
        "RR    "
        "BR    "
        "BR    "
        "BO    "
        "YO    "
        "YO    "
        "YO    ");

    bool found = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        if (actual != expected)
            return;
        EXPECT_FALSE(found);
        found = true;

        CoreField cf(original);
        EXPECT_TRUE(cf.dropPuyoList(cpl));
        EXPECT_EQ(expected, cf);
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByFloatStrategy(original, noProhibits, 3, 12, callback);

    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, detectByFloatStrategy2)
{
    const CoreField original(
        ".....G"
        ".....R"
        ".....R"
        ".....G"
        ".....G"
        ".....R"
        "....GR"
        "...GGR");

    const CoreField expected(
        ".....G"
        ".....R"
        ".....R"
        ".....G"
        "....GG"
        "....GR"
        "....GR"
        "...GGR");

    bool found = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        if (actual != expected)
            return;
        EXPECT_FALSE(found);
        found = true;

        CoreField cf(original);
        EXPECT_TRUE(cf.dropPuyoList(cpl));
        EXPECT_EQ(expected, cf);
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByFloatStrategy(original, noProhibits, 3, 12, callback);

    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, detectByFloatStrategy3)
{
    CoreField original(
        "....G."
        "....R."
        "....R."
        "....G."
        "....G."
        "....R."
        "...GR."
        "..GGR.");

    const CoreField expected[] {
        CoreField(
            "....G."
            "....R."
            "....R."
            "....G."
            "....GG"
            "....RG"
            "...GRO"
            "..GGRO"),
        CoreField(
            "....G."
            "....R."
            "....R."
            "....G."
            "...GG."
            "...GR."
            "...GR."
            "..GGR."),
    };

    bool found[ARRAY_SIZE(expected)] {};
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (expected[i] != actual)
                continue;
            EXPECT_FALSE(found[i]);
            found[i] = true;

            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected[i], cf);
        }
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByFloatStrategy(original, noProhibits, 3, 12, callback);

    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i)
        EXPECT_TRUE(found[i]);
}

TEST(RensaDetectorTest, detectByExtendStrategy1)
{
    const CoreField original(
        "..RRR.");

    const CoreField expected[] {
        CoreField(
            ".RRRR."),
        CoreField(
            "..R..."
            "..RRR."),
        CoreField(
            "...R.."
            "..RRR."),
        CoreField(
            "....R."
            "..RRR."),
        CoreField(
            "..RRRR")
    };

    bool found[ARRAY_SIZE(expected)] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        bool ok = false;
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (expected[i] != actual)
                continue;
            EXPECT_FALSE(found[i]);
            found[i] = true;

            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected[i], cf);
            ok = true;
            break;
        }

        if (!ok)
            unexpectedFound = true;
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByExtendStrategy(original, noProhibits, 1, 12, callback);

    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i)
        EXPECT_TRUE(found[i]);
    EXPECT_FALSE(unexpectedFound);
}

TEST(RensaDetectorTest, detectByExtendStrategy2)
{
    const CoreField original(
        "...R.."
        "..RR..");

    const CoreField expected[] {
        CoreField(
            "...R.."
            ".RRR.."),
        CoreField(
            "..RR.."
            "..RR.."),
        CoreField(
            "...R.."
            "...R.."
            "..RR.."),
        CoreField(
            "...R.."
            "..RRR."),
    };

    bool found[ARRAY_SIZE(expected)] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        bool ok = false;
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (expected[i] != actual)
                continue;
            EXPECT_FALSE(found[i]);
            found[i] = true;

            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected[i], cf);
            ok = true;
            break;
        }

        if (!ok)
            unexpectedFound = true;
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByExtendStrategy(original, noProhibits, 1, 12, callback);

    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i)
        EXPECT_TRUE(found[i]);
    EXPECT_FALSE(unexpectedFound);

}

TEST(RensaDetectorTest, detectByExtendStrategy3)
{
    const CoreField original(
        "...R.."
        "..RR.."
        "..OO..");

    const CoreField expected[] {
        CoreField(
            "..RR.."
            "..RR.."
            "..OO.."),
        CoreField(
            "...R.."
            "...R.."
            "..RR.."
            "..OO.."),
    };

    bool found[ARRAY_SIZE(expected)] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        bool ok = false;
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (expected[i] != actual)
                continue;
            EXPECT_FALSE(found[i]);
            found[i] = true;

            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected[i], cf);
            ok = true;
            break;
        }

        if (!ok) {
            unexpectedFound = true;
            FAIL() << actual;
        }
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByExtendStrategy(original, noProhibits, 1, 12, callback);

    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i)
        EXPECT_TRUE(found[i]);
    EXPECT_FALSE(unexpectedFound);
}

TEST(RensaDetectorTest, detectByExtendStrategy4)
{
    const CoreField original(
        "..RR.."
        "..OO..");

    const CoreField expected[] {
        CoreField(
            "..RR.."
            "..RR.."
            "..OO.."),
        CoreField(
            "...R.."
            "...R.."
            "..RR.."
            "..OO.."),
        CoreField(
            "..R..."
            "..R..."
            "..RR.."
            "..OO.."),
        CoreField(
            "..RRR."
            "..OOR."),
        CoreField(
            ".RRR.."
            ".ROO.."),
    };

    bool found[ARRAY_SIZE(expected)] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        bool ok = false;
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (expected[i] != actual)
                continue;
            EXPECT_FALSE(found[i]);
            found[i] = true;

            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected[i], cf);
            ok = true;
            break;
        }

        if (!ok)
            unexpectedFound = true;
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByExtendStrategy(original, noProhibits, 2, 12, callback);

    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i)
        EXPECT_TRUE(found[i]);
    EXPECT_FALSE(unexpectedFound);
}

TEST(RensaDetectorTest, detectByExtendStrategy5)
{
    const CoreField original(
        "..R..."
        "..OO..");

    const CoreField expected[] {
        CoreField(
            "..R..."
            "..R..."
            "..R..."
            "..R..."
            "..OO.."),
        CoreField(
            "..RR.."
            "..RR.."
            "..OO.."),
        CoreField(
            "..R..."
            "..R..."
            "..RR.."
            "..OO.."),
        CoreField(
            ".RRR.."
            ".ROO.."),
        CoreField(
            "...R.."
            "...R.."
            "..RR.."
            "..OO.."),
        CoreField(
            "..RRR."
            "..OOR."),
        CoreField(
            "..R..."
            ".RR..."
            ".ROO.."),
        CoreField(
            ".RR..."
            "RROO.."),
        CoreField(
            ".R...."
            ".RR..."
            ".ROO.."),
    };

    bool found[ARRAY_SIZE(expected)] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField& actual, const ColumnPuyoList& cpl) {
        bool ok = false;
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (expected[i] != actual)
                continue;
            EXPECT_FALSE(found[i]);
            found[i] = true;

            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected[i], cf);
            ok = true;
            break;
        }

        if (!ok)
            unexpectedFound = true;
    };

    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByExtendStrategy(original, noProhibits, 3, 12, callback);

    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i)
        EXPECT_TRUE(found[i]);
    EXPECT_FALSE(unexpectedFound);
}

TEST(RensaDetectorTest, detectIteratively_depth1_1)
{
    const CoreField original(
        "B     "
        "B     "
        "RGG   "
        "RBB   ");

    const CoreField expected(
        "BG    "
        "BG    "
        "RGG   "
        "RBB   ");

    const CoreField unexpected(
        "BR    "
        "BR    "
        "RGGG  "
        "RBBG  ");

    bool foundExpected = false;
    bool foundUnexpected = false;

    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl) -> RensaResult {
        if (complementedField == expected) {
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected, cf);
            foundExpected = true;
        }
        else if (complementedField == unexpected)
            foundUnexpected = true;
        return complementedField.simulate();
    };

    RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 1, callback);

    EXPECT_TRUE(foundExpected);
    EXPECT_FALSE(foundUnexpected);
}

TEST(RensaDetectorTest, detectIteratively_depth1_2)
{
    const CoreField original(
        " G    "
        " BR   "
        " GYR  "
        "BGGY  "
        "YYYR  ");

    const CoreField expected(
        " G    "
        "BBR   "
        "BGYR  "
        "BGGY  "
        "YYYR  ");

    bool foundExpected = false;

    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl) -> RensaResult {
        if (complementedField == expected) {
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected, cf);
            foundExpected = true;
        }
        return complementedField.simulate();

    };

    RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 1, callback);

    EXPECT_TRUE(foundExpected);
}

TEST(RensaDetectorTest, detectIteratively_depth2_1)
{
    const CoreField original(
        "B     "
        "B     "
        "RGGY  "
        "RBBG  ");

    const CoreField expected1(
        "BRG   "
        "BRG   "
        "RGGY  "
        "RBBG  ");

    const CoreField expected2(
        "B     "
        "B  GY "
        "RGGYY "
        "RBBGY ");

    const CoreField expected3(
        "R     "
        "R     "
        "BB    "
        "BB    "
        "RGGY  "
        "RBBG  ");

    const CoreField unexpected(
        "BR    "
        "BR GY "
        "RGGYY "
        "RBBGY ");

    bool foundExpected1 = false;
    bool foundExpected2 = false;
    bool foundExpected3 = false;
    bool foundUnexpected = false;

    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl) -> RensaResult {
        if (complementedField == expected1) {
            // Don't iterate the same one twice.
            EXPECT_FALSE(foundExpected1);
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected1, cf);
            foundExpected1 = true;
        } else if (complementedField == expected2) {
            EXPECT_FALSE(foundExpected2);
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected2, cf);
            foundExpected2 = true;
        } else if (complementedField == expected3) {
            EXPECT_FALSE(foundExpected3);
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected3, cf);
            foundExpected3 = true;
        } else if (complementedField == unexpected)
            foundUnexpected = true;
        return complementedField.simulate();
    };

    RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 2, callback);

    EXPECT_TRUE(foundExpected1);
    EXPECT_TRUE(foundExpected2);
    EXPECT_TRUE(foundExpected3);
    EXPECT_FALSE(foundUnexpected);
}

TEST(RensaDetectorTest, detectIteratively_depth2_2)
{
    const CoreField original(
        "  G   "
        "  Y   ");

    const CoreField expected(
        "  Y   "
        "  Y   "
        "  YG  "
        "  GG  "
        "  YG  ");

    bool foundExpected = false;
    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl) -> RensaResult {
        if (complementedField == expected) {
            // Don't iterate the same one twice.
            EXPECT_FALSE(foundExpected);
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected, cf);
            foundExpected = true;
        }
        return complementedField.simulate();
    };

    RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 2, callback);

    EXPECT_TRUE(foundExpected);
}

TEST(RensaDetectorTest, detectIteratively_depth3_1)
{
    const CoreField original(
        "  G   "
        "RBBBR ");

    const CoreField expected(
        " G R  "
        " GBR  "
        " GGR  "
        "RBBBR ");

    bool foundExpected = false;
    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl) -> RensaResult {
        if (complementedField == expected) {
            foundExpected = true;
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected, cf);
        }
        return complementedField.simulate();
    };

    RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 3, callback);

    EXPECT_TRUE(foundExpected);
}

TEST(RensaDetectorTest, detectIteratively_depth3_2)
{
    const CoreField original(
        "  R   "
        "GBB   ");

    const CoreField expected(
        " GB   "
        " GBR  "
        " GRR  "
        "GBBR  ");

    bool foundExpected = false;
    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl) -> RensaResult {
        if (complementedField == expected) {
            foundExpected = true;
            CoreField cf(original);
            EXPECT_TRUE(cf.dropPuyoList(cpl));
            EXPECT_EQ(expected, cf);
        }
        return complementedField.simulate();
    };

    RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 3, callback);

    EXPECT_TRUE(foundExpected);
}

TEST(RensaDetectorTest, detectIteratively_DontCrash)
{
    const CoreField original;
    auto callback = [](CoreField&& complementedField, const ColumnPuyoList&) -> RensaResult {
        return complementedField.simulate();
    };
    RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 2, callback);
}

TEST(RensaDetectorTest, complementKeyPuyosOn13thRow1)
{
    CoreField original;
    bool called = false;
    auto callback = [&called](CoreField&& cf, const ColumnPuyoList& cpl) {
        called = true;
        EXPECT_EQ(CoreField(), cf);
        EXPECT_EQ(0, cpl.size());
    };

    bool allowsComplements[FieldConstant::MAP_WIDTH];
    std::fill(allowsComplements, allowsComplements + FieldConstant::MAP_WIDTH, true);
    RensaDetector::complementKeyPuyosOn13thRow(original, allowsComplements, callback);
    EXPECT_TRUE(called);
}

TEST(RensaDetectorTest, complementKeyPuyosOn13thRow2)
{
    CoreField original(
        "OO.OOO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    int numCalled = 0;
    auto callback = [&numCalled](CoreField&& /*cf*/, const ColumnPuyoList& /*cpl*/) {
        ++numCalled;
    };

    bool allowsComplements[FieldConstant::MAP_WIDTH];
    std::fill(allowsComplements, allowsComplements + FieldConstant::MAP_WIDTH, true);
    RensaDetector::complementKeyPuyosOn13thRow(original, allowsComplements, callback);
    EXPECT_EQ(5 * 5 * 5 * 5 * 5, numCalled);
}

TEST(RensaDetectorTest, detectSideChain)
{
    const CoreField original(
        "B....."
        "BY...."
        "RR...."
        "BYR..."
        "BYR...");

    const CoreField expected(
        "BY...."
        "BY...."
        "RRR..."
        "BYR..."
        "BYR...");

    bool found = false;
    auto callback = [&](CoreField&& cf, const ColumnPuyoList& /*cpl*/) {
        if (cf == expected)
            found = true;
    };

    RensaDetector::detectSideChain(original, RensaDetectorStrategy::defaultDropStrategy(), callback);
    EXPECT_TRUE(found);
}
