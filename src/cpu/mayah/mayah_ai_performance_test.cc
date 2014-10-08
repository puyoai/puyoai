#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "base/time_stamp_counter.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/field/core_field.h"
#include "core/frame_request.h"
#include "core/kumipuyo_seq.h"

#include "mayah_ai.h"

using namespace std;

unique_ptr<MayahAI> makeAI()
{
    int argc = 1;
    char arg[] = "mayah";
    char* argv[] = {arg, nullptr};

    MayahAI* ai = new MayahAI(argc, argv);

    // TODO(mayah): should call gameWillBegin.
    FrameRequest req;
    req.frameId = 1;
    ai->onGameWillBegin(req);

    return unique_ptr<MayahAI>(ai);
}

CoreField fulfilledField() {
    return CoreField(
            "G   YG"
            "R   YY"
            "GRGYRG"
            "RGYRGG"
            "GRGYRY"
            "GRGYRY"
            "GYRGYR"
            "RGYRGY"
            "RGYRGY"
            "RGYRGY");
}

KumipuyoSeq defaultKumipuyoSeq(int n)
{
    switch (n) {
    case 2:
        return KumipuyoSeq("RRGG");
    case 3:
        return KumipuyoSeq("RRGGYY");
    case 4:
        return KumipuyoSeq("RRGGYYBB");
    default:
        CHECK(false) << n;
    }
}

void runTest(int depth, int iteration, const CoreField& cf, const KumipuyoSeq& kumipuyoSeq)
{
    TsumoPossibility::initialize();
    TimeStampCounterData tsc;

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;

    for (int i = 0; i < 3; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, AdditionalThoughtInfo(), depth, iteration);
    }

    tsc.showStatistics();
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter2)
{
    runTest(2, 2, CoreField(), defaultKumipuyoSeq(2));
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter2_fulfilled)
{
    runTest(2, 2, fulfilledField(), defaultKumipuyoSeq(2));
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter3)
{
    runTest(2, 3, CoreField(), defaultKumipuyoSeq(2));
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter3_fulfilled)
{
    runTest(2, 3, fulfilledField(), defaultKumipuyoSeq(2));
}

TEST(MayahAIPerformanceTest, seq2_depth3_iter1)
{
    runTest(3, 1, CoreField(), defaultKumipuyoSeq(2));
}

TEST(MayahAIPerformanceTest, seq2_depth3_iter1_fulfilled)
{
    runTest(3, 1, fulfilledField(), defaultKumipuyoSeq(2));
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter1)
{
    runTest(3, 1, CoreField(), defaultKumipuyoSeq(3));
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter1_fulfilled)
{
    runTest(3, 1, fulfilledField(), defaultKumipuyoSeq(3));
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter2)
{
    runTest(3, 2, CoreField(), defaultKumipuyoSeq(3));
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter2_fulfilled)
{
    runTest(3, 2, fulfilledField(), defaultKumipuyoSeq(3));
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter3)
{
    runTest(3, 3, CoreField(), defaultKumipuyoSeq(3));
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter3_fulfilled)
{
    runTest(3, 3, fulfilledField(), defaultKumipuyoSeq(3));
}

TEST(MayahAIPerformanceTest, from_real)
{
    CoreField f(
        "    RB"
        " B GGG"
        "GG YBR"
        "YG YGR"
        "GBYBGR"
        "BBYYBG"
        "GYBGRG"
        "GGYGGR"
        "YYBBBR");
    KumipuyoSeq seq("RBRGRYYG");

    runTest(3, 2, f, seq);
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    google::ParseCommandLineFlags(&argc, &argv, true);

    return RUN_ALL_TESTS();
}

