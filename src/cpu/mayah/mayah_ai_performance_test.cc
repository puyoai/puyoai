#include "core/algorithm/plan.h"

#include <iostream>
#include <gtest/gtest.h>

#include "base/tsc.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"

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

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 3;
    int numIteration = 1;

    for (int i = 0; i < 3; ++i) {
        Tsc tsc("depth3_iter1");
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, depth, numIteration);
    }

    double average, variance;
    Tsc::GetStatistics("depth3_iter1", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(MayahAIPerformanceTest, depth2_iter2)
{
    TsumoPossibility::initialize();

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 2;
    int numIteration = 2;

    for (int i = 0; i < 3; ++i) {
        Tsc tsc("depth2_iter2");
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, depth, numIteration);
    }

    double average, variance;
    Tsc::GetStatistics("depth2_iter2", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(MayahAIPerformanceTest, depth2_iter3)
{
    TsumoPossibility::initialize();

    unique_ptr<MayahAI> ai(makeAI());
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 2;
    int numIteration = 3;

    for (int i = 0; i < 3; ++i) {
        Tsc tsc("depth2_iter3");
        (void)ai->thinkPlan(frameId, cf, kumipuyoSeq, depth, numIteration);
    }

    double average, variance;
    Tsc::GetStatistics("depth2_iter3", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

