#include "core/algorithm/rensa_detector.h"

#include "core/algorithm/rensa_ref_sequence.h"
#include "core/kumipuyo_seq.h"

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

TEST(RensaDetectorTest, feasibleRensas)
{
    CoreField f(
        "BRGY  "
        "BBRGG "
        "RRGYYY");
    KumipuyoSeq seq("BYYY");

    vector<FeasibleRensaInfo> infos = RensaDetector::findFeasibleRensas(f, seq);

    bool found = false;
    for (const auto info : infos) {
        if (info.chains() == 4 && info.framesToInitiate() == 0) {
            found = true;
        }
    }

    EXPECT_TRUE(found);
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

    RensaDetector::iteratePossibleRensas(f, 3, RensaDetectorStrategy::defaultDropStrategy(), callback);

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

    RensaDetector::iteratePossibleRensas(f, 0, RensaDetectorStrategy::defaultFloatStrategy(), callback);
    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, iteratePossibleRensasFloat2)
{
    CoreField f(
        ".....G"
        ".....R"
        ".....R"
        ".....G"
        ".....G"
        ".....R"
        "....GR"
        "...GGR");

    bool found = false;
    ColumnPuyoList expectedFireList;
    expectedFireList.add(5, PuyoColor::GREEN);
    expectedFireList.add(5, PuyoColor::GREEN);
    auto callback = [&](const CoreField& /*fieldAfterRensa*/, const RensaResult& rensaResult,
                        const ColumnPuyoList& /*keyPuyos*/, const ColumnPuyoList& firePuyos) {
        if (rensaResult.chains == 3 && firePuyos == expectedFireList) {
            found = true;
        }
    };

    RensaDetector::iteratePossibleRensas(f, 0, RensaDetectorStrategy::defaultFloatStrategy(), callback);
    EXPECT_TRUE(found);
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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&, const RensaRefSequence&) {
        CoreField g(f);
        for (const auto& cp : keyPuyos)
            g.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            g.dropPuyoOn(cp.x, cp.color);

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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&, const RensaRefSequence&) {
        CoreField g(f);
        for (const auto& cp : keyPuyos)
            g.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            g.dropPuyoOn(cp.x, cp.color);

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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&, const RensaRefSequence&) {
        CoreField g(f);
        for (const auto& cp : keyPuyos)
            g.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            g.dropPuyoOn(cp.x, cp.color);

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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&, const RensaRefSequence&) {
        CoreField g(f);
        for (const auto& cp : keyPuyos)
            g.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            g.dropPuyoOn(cp.x, cp.color);

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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&, const RensaRefSequence&) {
        CoreField g(f);
        for (const auto& cp : keyPuyos)
            g.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            g.dropPuyoOn(cp.x, cp.color);

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
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult&, const RensaRefSequence&) {

        CoreField g(f);
        for (const auto& cp : keyPuyos)
            g.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            g.dropPuyoOn(cp.x, cp.color);

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
                       const ColumnPuyoList&, const ColumnPuyoList&,
                       const RensaTrackResult&, const RensaRefSequence&) {
    };
    RensaDetector::iteratePossibleRensasIteratively(f, 2, RensaDetectorStrategy::defaultDropStrategy(), callback);
}
