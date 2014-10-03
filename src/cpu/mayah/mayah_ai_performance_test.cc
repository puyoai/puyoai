#include <iostream>
#include <gtest/gtest.h>

#include "base/time_stamp_counter.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/field/core_field.h"
#include "core/kumipuyo_seq.h"

#include "mayah_ai.h"

using namespace std;

unique_ptr<MayahAI> makeAI()
{
    int argc = 1;
    char arg[] = "mayah";
    char* argv[] = {arg};
    return unique_ptr<MayahAI>(new MayahAI(argc, argv));
}

TEST(MayahAIPerformanceTest, depth3_iter1)
{
    TsumoPossibility::initialize();
    TimeStampCounterData tsc;

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 3;
    int numIteration = 1;

    for (int i = 0; i < 3; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, AdditionalThoughtInfo(), depth, numIteration);
    }

    tsc.showStatistics();
}

TEST(MayahAIPerformanceTest, depth2_iter2)
{
    TsumoPossibility::initialize();
    TimeStampCounterData tsc;

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 2;
    int numIteration = 2;

    for (int i = 0; i < 3; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, AdditionalThoughtInfo(), depth, numIteration);
    }

    tsc.showStatistics();
}

TEST(MayahAIPerformanceTest, depth2_iter3)
{
    TsumoPossibility::initialize();
    TimeStampCounterData tsc;

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 2;
    int numIteration = 3;

    for (int i = 0; i < 3; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, AdditionalThoughtInfo(), depth, numIteration);
    }

    tsc.showStatistics();
}

