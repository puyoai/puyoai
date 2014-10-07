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
    char* argv[] = {arg};

    MayahAI* ai = new MayahAI(argc, argv);

    // TODO(mayah): should call gameWillBegin.
    FrameRequest req;
    req.frameId = 1;
    ai->onGameWillBegin(req);

    return unique_ptr<MayahAI>(ai);
}

void runTest(int seqSize, int depth, int iteration, bool fulfilled)
{
    TsumoPossibility::initialize();
    TimeStampCounterData tsc;

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;

    CoreField cf;
    if (fulfilled) {
        cf = CoreField(
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


    KumipuyoSeq kumipuyoSeq;
    if (seqSize == 2) {
        kumipuyoSeq = KumipuyoSeq("RRGG");
    } else if (seqSize == 3) {
        kumipuyoSeq = KumipuyoSeq("RRGGYY");
    } else if (seqSize == 4) {
        kumipuyoSeq = KumipuyoSeq("RRGGYYBB");
    } else {
        CHECK(false) << seqSize;
    }

    for (int i = 0; i < 3; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, AdditionalThoughtInfo(), depth, iteration);
    }

    tsc.showStatistics();
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter2)
{
    runTest(2, 2, 2, false);
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter2_fulfilled)
{
    runTest(2, 2, 2, true);
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter3)
{
    runTest(2, 2, 3, false);
}

TEST(MayahAIPerformanceTest, seq2_depth2_iter3_fulfilled)
{
    runTest(2, 2, 3, true);
}

TEST(MayahAIPerformanceTest, seq2_depth3_iter1)
{
    runTest(2, 3, 1, false);
}

TEST(MayahAIPerformanceTest, seq2_depth3_iter1_fulfilled)
{
    runTest(2, 3, 1, true);
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter1)
{
    runTest(3, 3, 1, false);
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter1_fulfilled)
{
    runTest(3, 3, 1, true);
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter2)
{
    runTest(3, 3, 2, false);
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter2_fulfilled)
{
    runTest(3, 3, 2, true);
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter3)
{
    runTest(3, 3, 3, false);
}

TEST(MayahAIPerformanceTest, seq3_depth3_iter3_fulfilled)
{
    runTest(3, 3, 3, true);
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    google::ParseCommandLineFlags(&argc, &argv, true);

    return RUN_ALL_TESTS();
}

