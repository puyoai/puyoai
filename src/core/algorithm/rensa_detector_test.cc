#include "core/algorithm/rensa_detector.h"

#include "core/algorithm/rensa_ref_sequence.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace std;

static void dropKeyAndFirePuyos(CoreField* f, const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos)
{
    for (const auto& p : keyPuyos) {
        f->dropPuyoOn(p.x, p.color);
    }
    for (const auto& p : firePuyos) {
        f->dropPuyoOn(p.x, p.color);
    }
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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        UNUSED_VARIABLE(fieldAfterRensa);
        UNUSED_VARIABLE(rensaResult);
        CoreField actual(f);
        dropKeyAndFirePuyos(&actual, keyPuyos, firePuyos);
        if (actual == expected1)
            found1 = true;
        if (actual == expected2)
            found2 = true;
    };

    RensaDetector::iteratePossibleRensas(f, 0, callback);
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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        UNUSED_VARIABLE(fieldAfterRensa);
        UNUSED_VARIABLE(rensaResult);
        CoreField actual(f);
        dropKeyAndFirePuyos(&actual, keyPuyos, firePuyos);
        for (int i = 0; i < 4; ++i) {
            if (actual == expected[i]) {
                found[i] = true;
                break;
            }
        }
    };

    RensaDetector::iteratePossibleRensas(f, 3, callback);

    EXPECT_TRUE(found[0]);
    EXPECT_TRUE(found[1]);
    EXPECT_TRUE(found[2]);
    EXPECT_TRUE(found[3]);
}

TEST(RensaDetectorTest, iteratePossibleRensasFloat)
{
    CoreField f("y     "
                "b     "
                "r     "
                "b     "
                "b     "
                "b     "
                "y     "
                "y     "
                "y     ");

    CoreField g("y     "
                "b     "
                "rr    "
                "br    "
                "br    "
                "bO    "
                "yO    "
                "yO    "
                "yO    ");

    auto expected = g.simulate();

    bool found = false;
    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        UNUSED_VARIABLE(fieldAfterRensa);
        UNUSED_VARIABLE(keyPuyos);
        UNUSED_VARIABLE(firePuyos);
        if (rensaResult == expected)
            found = true;
    };

    RensaDetector::iteratePossibleRensas(f, 0, callback, RensaDetector::Mode::FLOAT);
    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively)
{
    CoreField f(
        "B     "
        "B     "
        "RGG   "
        "RBB   ");

    CoreField expected1(
        "BR    "
        "BR    "
        "RGGG  "
        "RBBG  ");

    CoreField expected2(
        "BRG   "
        "BRG   "
        "RGG   "
        "RBB   ");

    bool foundExpected1 = false;
    bool foundExpected2 = false;

    auto callback = [&](const RensaRefSequence& seq) {
        CoreField g(f);
        for (const auto& p : seq.combinedRensa().keyPuyos)
            g.dropPuyoOn(p.x, p.color);
        for (const auto& p : seq.combinedRensa().firePuyos)
            g.dropPuyoOn(p.x, p.color);

        if (g == expected1)
            foundExpected1 = true;
        else if (g == expected2)
            foundExpected2 = true;
    };

    RensaDetector::iteratePossibleRensasIteratively(f, 2, callback);

    EXPECT_TRUE(foundExpected1);
    EXPECT_TRUE(foundExpected1);
}

TEST(RensaDetectorTest, iteratePossibleRensasIteratively_DontCrash)
{
    CoreField f;
    auto callback = [&](const RensaRefSequence&) { };
    RensaDetector::iteratePossibleRensasIteratively(f, 2, callback);
}
