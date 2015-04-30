#include "core/algorithm/rensa_detector.h"

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

TEST(RensaDetectorTest, detect1)
{
    CoreField f(
        "..RRR.");

    pair<CoreField, ColumnPuyoList> expected[5];
    {
        expected[0].first = CoreField(
            ".RRRR.");
        expected[0].second.add(2, PuyoColor::RED, 1);
    }
    {
        expected[1].first = CoreField(
            "..R..."
            "..RRR.");
        expected[1].second.add(3, PuyoColor::RED, 1);
    }
    {
        expected[2].first = CoreField(
            "...R.."
            "..RRR.");
        expected[2].second.add(4, PuyoColor::RED, 1);
    }
    {
        expected[3].first = CoreField(
            "....R."
            "..RRR.");
        expected[3].second.add(5, PuyoColor::RED, 1);
    }
    {
        expected[4].first = CoreField(
            "..RRRR");
        expected[4].second.add(6, PuyoColor::RED, 1);
    }

    bool found[5] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField& complementedField, const ColumnPuyoList& firePuyos) {
        bool ok = false;
        for (int i = 0; i < 5; ++i) {
            if (expected[i].first == complementedField && expected[i].second == firePuyos) {
                found[i] = true;
                ok = true;
                break;
            }
        }

        if (!ok)
            unexpectedFound = true;
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::EXTEND, 0, 1, false);
    static const bool nonProhibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detect(f, strategy, PurposeForFindingRensa::FOR_FIRE, nonProhibits, callback);
    EXPECT_FALSE(unexpectedFound);
    EXPECT_TRUE(found[0]);
    EXPECT_TRUE(found[1]);
    EXPECT_TRUE(found[2]);
    EXPECT_TRUE(found[3]);
    EXPECT_TRUE(found[4]);
}

TEST(RensaDetectorTest, detectDrop1)
{
    CoreField f(
        ".R...."
        "RB...."
        "RB...."
        "RB...."
        "BR...."
        "BR...."
        "BRGYG."
        "RGYGB."
        "RGYGB."
        "RGYGB.");

    CoreField expected(
        ".R...."
        "RB...."
        "RB...."
        "RB...."
        "BR...."
        "BRR..."
        "BRGYG."
        "RGYGB."
        "RGYGB."
        "RGYGB.");

    bool found = false;
    auto callback = [&](const CoreField&, const RensaResult&, const ColumnPuyoList& cpl) {
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        if (actual == expected) {
            found = true;
        }
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::DROP, 2, 2, false);
    RensaDetector::iteratePossibleRensas(f, 0, strategy, callback);
    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, detectExtend1)
{
    CoreField f(
        "..RRR.");

    CoreField expected[] {
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

    bool found[5] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField&, const RensaResult&, const ColumnPuyoList& cpl) {
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        bool ok = false;
        for (int i = 0; i < 5; ++i) {
            if (expected[i] == actual) {
                found[i] = true;
                ok = true;
                break;
            }
        }

        if (!ok)
            unexpectedFound = true;
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::EXTEND, 0, 1, false);
    RensaDetector::iteratePossibleRensas(f, 0, strategy, callback);
    EXPECT_FALSE(unexpectedFound);
    EXPECT_TRUE(found[0]);
    EXPECT_TRUE(found[1]);
    EXPECT_TRUE(found[2]);
    EXPECT_TRUE(found[3]);
    EXPECT_TRUE(found[4]);
}

TEST(RensaDetectorTest, detectExtend2)
{
    CoreField f(
        "...R.."
        "..RR..");

    CoreField expected[] {
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

    bool found[4] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField&, const RensaResult&, const ColumnPuyoList& cpl) {
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        bool ok = false;
        for (int i = 0; i < 4; ++i) {
            if (expected[i] == actual) {
                found[i] = true;
                ok = true;
                break;
            }
        }

        if (!ok)
            unexpectedFound = true;
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::EXTEND, 0, 1, false);
    RensaDetector::iteratePossibleRensas(f, 0, strategy, callback);
    EXPECT_FALSE(unexpectedFound);
    EXPECT_TRUE(found[0]);
    EXPECT_TRUE(found[1]);
    EXPECT_TRUE(found[2]);
    EXPECT_TRUE(found[3]);
}

TEST(RensaDetectorTest, detectExtend3)
{
    CoreField f(
        "...R.."
        "..RR.."
        "..OO..");

    CoreField expected[] {
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

    bool found[2] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField&, const RensaResult&, const ColumnPuyoList& cpl) {
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        bool ok = false;
        for (int i = 0; i < 2; ++i) {
            if (expected[i] == actual) {
                found[i] = true;
                ok = true;
                break;
            }
        }

        if (!ok)
            unexpectedFound = true;
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::EXTEND, 0, 1, false);
    RensaDetector::iteratePossibleRensas(f, 0, strategy, callback);
    EXPECT_FALSE(unexpectedFound);
    EXPECT_TRUE(found[0]);
    EXPECT_TRUE(found[1]);
}

TEST(RensaDetectorTest, detectExtend4)
{
    CoreField f(
        "..RR.."
        "..OO..");

    CoreField expected[] {
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

    const size_t N = ARRAY_SIZE(expected);

    bool found[N] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField&, const RensaResult&, const ColumnPuyoList& cpl) {
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        bool ok = false;
        for (size_t i = 0; i < N; ++i) {
            if (expected[i] == actual) {
                found[i] = true;
                ok = true;
                break;
            }
        }

        if (!ok)
            unexpectedFound = true;
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::EXTEND, 0, 2, false);
    RensaDetector::iteratePossibleRensas(f, 0, strategy, callback);
    EXPECT_FALSE(unexpectedFound);
    for (size_t i = 0; i < N; ++i)
        EXPECT_TRUE(found[i]) << i;
}

TEST(RensaDetectorTest, detectExtend5)
{
    CoreField f(
        "..R..."
        "..OO..");

    CoreField expected[] {
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

    const size_t N = ARRAY_SIZE(expected);

    bool found[N] {};
    bool unexpectedFound = false;
    auto callback = [&](const CoreField&, const RensaResult&, const ColumnPuyoList& cpl) {
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        bool ok = false;
        for (size_t i = 0; i < N; ++i) {
            if (expected[i] == actual) {
                found[i] = true;
                ok = true;
                break;
            }
        }

        if (!ok) {
            cout << actual.toDebugString() << endl;
            unexpectedFound = true;
        }
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::EXTEND, 0, 3, false);
    RensaDetector::iteratePossibleRensas(f, 0, strategy, callback);
    EXPECT_FALSE(unexpectedFound);
    for (size_t i = 0; i < N; ++i)
        EXPECT_TRUE(found[i]) << i;
}

TEST(RensaDetectorTest, iteratePossibleRensa)
{
    CoreField f(" BRR  "
                " RBR  "
                "BBYYY ");

    CoreField expected1(" BRR  "
                        " RBR  "
                        "BBYYYY");

    CoreField expected2(" BRR  "
                        " RBRY "
                        "BBYYY ");

    bool found1 = false;
    bool found2 = false;
    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& cpl) {
        UNUSED_VARIABLE(fieldAfterRensa);
        UNUSED_VARIABLE(rensaResult);
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        if (actual == expected1)
            found1 = true;
        if (actual == expected2)
            found2 = true;
    };

    RensaDetector::iteratePossibleRensas(f, 0, RensaDetectorStrategy::defaultDropStrategy(), callback);
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
}

TEST(RensaDetectorTest, iteratePossibleRensaWithKeyPuyos)
{
    CoreField f("RB    "
                "RRB   "
                "BBY   ");

    CoreField expected[] {
        CoreField("R     "
                  "RBY   "
                  "RRBY  "
                  "BBYY  "),
        CoreField("R     "
                  "RBY   "
                  "RRB   "
                  "BBYYY "),
        CoreField("RY    "
                  "RBY   "
                  "RRB   "
                  "BBYY  "),
        CoreField("RY    "
                  "RB    "
                  "RRB   "
                  "BBYYY ")
    };

    bool found[4] {};
    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& cpl) {
        UNUSED_VARIABLE(fieldAfterRensa);
        UNUSED_VARIABLE(rensaResult);
        CoreField actual(f);
        EXPECT_TRUE(actual.dropPuyoList(cpl));

        for (int i = 0; i < 4; ++i) {
            if (actual == expected[i]) {
                found[i] = true;
                break;
            }
        }
    };

    RensaDetector::iteratePossibleRensas(f, 3, RensaDetectorStrategy::defaultDropStrategy(), callback);

    EXPECT_TRUE(found[0]);
    EXPECT_TRUE(found[1]);
    EXPECT_TRUE(found[2]);
    EXPECT_TRUE(found[3]);
}

TEST(RensaDetectorTest, iteratePossibleRensasFloat)
{
    CoreField cf("y     "
                 "b     "
                 "r     "
                 "b     "
                 "b     "
                 "b     "
                 "y     "
                 "y     "
                 "y     ");

    CoreField expected("y     "
                       "b     "
                       "rr    "
                       "br    "
                       "br    "
                       "bO    "
                       "yO    "
                       "yO    "
                       "yO    ");

    bool found = false;
    auto callback = [&](const CoreField& /*fieldAfterRensa*/, const RensaResult& /*rensaResult*/,
                        const ColumnPuyoList& cpl) {
        CoreField actual(cf);
        if (!actual.dropPuyoList(cpl))
            return;
        if (actual == expected)
            found = true;
    };

    RensaDetector::iteratePossibleRensas(cf, 0, RensaDetectorStrategy::defaultFloatStrategy(), callback);
    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, iteratePossibleRensasFloat2)
{
    CoreField cf(
        ".....G"
        ".....R"
        ".....R"
        ".....G"
        ".....G"
        ".....R"
        "....GR"
        "...GGR");

    CoreField expected(
        ".....G"
        ".....R"
        ".....R"
        ".....G"
        "....GG"
        "....GR"
        "....GR"
        "...GGR");

    bool found = false;
    auto callback = [&](const CoreField& /*fieldAfterRensa*/, const RensaResult& /*rensaResult*/,
                        const ColumnPuyoList& cpl) {
        CoreField actual(cf);
        if (!actual.dropPuyoList(cpl))
            return;
        if (actual == expected)
            found = true;
    };


    RensaDetector::iteratePossibleRensas(cf, 0, RensaDetectorStrategy::defaultFloatStrategy(), callback);
    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, iteratePossibleRensasExtend1)
{
    CoreField f(
        "....R."
        "..RGGY");

    CoreField expected[] {
        CoreField("....G."
                  "....G."
                  "..RRR."
                  "..RGGY"),
        CoreField("......"
                  "....G."
                  "..RRRG"
                  "..RGGY"),
    };

    const int N = ARRAY_SIZE(expected);

    bool found[N] {};
    auto callback = [&](const CoreField&, const RensaResult&,
                        const ColumnPuyoList& cpl,
                        const RensaChainTrackResult&) {
        CoreField g(f);
        EXPECT_TRUE(g.dropPuyoList(cpl));

        for (int i = 0; i < N; ++i) {
            if (g == expected[i])
                found[i] = true;
        }
    };

    RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::EXTEND, 3, 3, false);
    RensaDetector::iteratePossibleRensasIteratively(f, 2, RensaDetectorStrategy::defaultExtendStrategy(), callback);

    for (int i = 0; i < N; ++i) {
        EXPECT_TRUE(found[i]) << i;
    }
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_depth1_1)
{
    CoreField f(
        "B     "
        "B     "
        "RGG   "
        "RBB   ");

    CoreField expected(
        "BG    "
        "BG    "
        "RGG   "
        "RBB   ");

    CoreField unexpected(
        "BR    "
        "BR    "
        "RGGG  "
        "RBBG  ");

    bool foundExpected = false;
    bool foundUnexpected = false;

    auto callback = [&](const CoreField&, const RensaResult&,
                        const ColumnPuyoList& cpl,
                        const RensaChainTrackResult&) {
        CoreField g(f);
        EXPECT_TRUE(g.dropPuyoList(cpl));

        if (g == expected)
            foundExpected = true;
        else if (g == unexpected)
            foundUnexpected = true;
    };

    RensaDetector::iteratePossibleRensasIteratively(f, 1, RensaDetectorStrategy::defaultDropStrategy(), callback);

    EXPECT_TRUE(foundExpected);
    EXPECT_FALSE(foundUnexpected);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_depth1_2)
{
    CoreField f(
        " G    "
        " BR   "
        " GYR  "
        "BGGY  "
        "YYYR  ");

    CoreField expected(
        " G    "
        "BBR   "
        "BGYR  "
        "BGGY  "
        "YYYR  ");

    bool foundExpected = false;

    auto callback = [&](const CoreField&, const RensaResult&,
                        const ColumnPuyoList& cpl,
                        const RensaChainTrackResult&) {
        CoreField g(f);
        EXPECT_TRUE(g.dropPuyoList(cpl));

        if (g == expected)
            foundExpected = true;
    };

    RensaDetector::iteratePossibleRensasIteratively(f, 1, RensaDetectorStrategy::defaultDropStrategy(), callback);

    EXPECT_TRUE(foundExpected);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_depth2_1)
{
    CoreField f(
        "B     "
        "B     "
        "RGGY  "
        "RBBG  ");

    CoreField expected1(
        "BRG   "
        "BRG   "
        "RGGY  "
        "RBBG  ");

    CoreField expected2(
        "B     "
        "B  GY "
        "RGGYY "
        "RBBGY ");

    CoreField expected3(
        "R     "
        "R     "
        "BB    "
        "BB    "
        "RGGY  "
        "RBBG  ");

    CoreField unexpected(
        "BR    "
        "BR GY "
        "RGGYY "
        "RBBGY ");

    bool foundExpected1 = false;
    bool foundExpected2 = false;
    bool foundExpected3 = false;
    bool foundUnexpected = false;

    auto callback = [&](const CoreField&, const RensaResult&,
                        const ColumnPuyoList& cpl,
                        const RensaChainTrackResult&) {
        CoreField g(f);
        EXPECT_TRUE(g.dropPuyoList(cpl));

        if (g == expected1) {
            // Don't iterate the same one twice.
            EXPECT_FALSE(foundExpected1);
            foundExpected1 = true;
        } else if (g == expected2) {
            EXPECT_FALSE(foundExpected2);
            foundExpected2 = true;
        } else if (g == expected3) {
            EXPECT_FALSE(foundExpected3);
            foundExpected3 = true;
        } else if (g == unexpected)
            foundUnexpected = true;
    };

    RensaDetector::iteratePossibleRensasIteratively(f, 2, RensaDetectorStrategy::defaultDropStrategy(), callback);

    EXPECT_TRUE(foundExpected1);
    EXPECT_TRUE(foundExpected2);
    EXPECT_TRUE(foundExpected3);
    EXPECT_FALSE(foundUnexpected);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_depth2_2)
{
    CoreField f(
        "  G   "
        "  Y   ");

    CoreField expected1(
        "  Y   "
        "  Y   "
        "  YG  "
        "  GG  "
        "  YG  ");

    bool foundExpected1 = false;

    auto callback = [&](const CoreField&, const RensaResult&,
                        const ColumnPuyoList& cpl,
                        const RensaChainTrackResult&) {
        CoreField g(f);
        EXPECT_TRUE(g.dropPuyoList(cpl));

        if (g == expected1) {
            // Don't iterate the same one twice.
            EXPECT_FALSE(foundExpected1);
            foundExpected1 = true;
        }
    };

    RensaDetector::iteratePossibleRensasIteratively(f, 2, RensaDetectorStrategy::defaultDropStrategy(), callback);

    EXPECT_TRUE(foundExpected1);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_depth3_1)
{
    CoreField f(
        "  G   "
        "RBBBR ");

    CoreField expected(
        " G R  "
        " GBR  "
        " GGR  "
        "RBBBR ");

    bool foundExpected = false;
    auto callback = [&](const CoreField&, const RensaResult&,
                        const ColumnPuyoList& cpl,
                        const RensaChainTrackResult&) {
        CoreField g(f);
        EXPECT_TRUE(g.dropPuyoList(cpl));

        if (g == expected)
            foundExpected = true;
    };

    RensaDetector::iteratePossibleRensasIteratively(f, 3, RensaDetectorStrategy::defaultDropStrategy(), callback);

    EXPECT_TRUE(foundExpected);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_depth3_2)
{
    CoreField f(
        "  R   "
        "GBB   ");

    CoreField expected(
        " GB   "
        " GBR  "
        " GRR  "
        "GBBR  ");

    bool foundExpected = false;
    auto callback = [&](const CoreField&, const RensaResult&,
                        const ColumnPuyoList& cpl,
                        const RensaChainTrackResult&) {

        CoreField g(f);
        EXPECT_TRUE(g.dropPuyoList(cpl));

        if (g == expected)
            foundExpected = true;
    };

    RensaDetector::iteratePossibleRensasIteratively(f, 3, RensaDetectorStrategy::defaultDropStrategy(), callback);

    EXPECT_TRUE(foundExpected);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_DontCrash)
{
    CoreField f;
    auto callback = [](const CoreField&, const RensaResult&,
                       const ColumnPuyoList&,
                       const RensaChainTrackResult&) {
    };
    RensaDetector::iteratePossibleRensasIteratively(f, 2, RensaDetectorStrategy::defaultDropStrategy(), callback);
}

TEST(RensaDetectorTest, complementKeyPuyos)
{
    CoreField original("...RRR");

    CoreField expected[] {
        CoreField("...RRR"),
        CoreField("..BRRR"),
        CoreField("..YRRR"),
        CoreField("..GRRR"),
        CoreField("R..RRR")
    };
    bool found[ARRAY_SIZE(expected)] {};

    auto callback = [&original, &expected, &found](const CoreField& cf, const ColumnPuyoList& cpl) {
        for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
            if (cf == expected[i]) {
                found[i] = true;
                CoreField f(original);
                EXPECT_TRUE(f.dropPuyoList(cpl));
                EXPECT_EQ(f, expected[i]);
            }
        }
    };
    RensaDetector::complementKeyPuyos(original, RensaDetectorStrategy::defaultDropStrategy(), 1, callback);

    for (size_t i = 0; i < ARRAY_SIZE(expected); ++i) {
        EXPECT_TRUE(found[i]);
    }
}
