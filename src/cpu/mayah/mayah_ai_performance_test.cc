#include "core/algorithm/plan.h"

#include <iostream>
#include <gtest/gtest.h>

#include "base/tsc.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"

#include "mayah_ai.h"

using namespace std;

TEST(MayahAIPerformanceTest, depth3_key0)
{
    TsumoPossibility::initialize();

    MayahAI ai;
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 3;
    int numKeyPuyos = 0;

    for (int i = 0; i < 3; ++i) {
        Tsc tsc("depth3_key0");
        (void)ai.thinkPlan(frameId, cf, kumipuyoSeq, depth, numKeyPuyos);
    }

    double average, variance;
    Tsc::GetStatistics("depth3_key0", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(MayahAIPerformanceTest, depth2_key1)
{
    TsumoPossibility::initialize();

    MayahAI ai;
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 2;
    int numKeyPuyos = 1;

    for (int i = 0; i < 3; ++i) {
        Tsc tsc("depth2_key1");
        (void)ai.thinkPlan(frameId, cf, kumipuyoSeq, depth, numKeyPuyos);
    }

    double average, variance;
    Tsc::GetStatistics("depth2_key1", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(MayahAIPerformanceTest, depth2_key2)
{
    TsumoPossibility::initialize();

    MayahAI ai;
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 2;
    int numKeyPuyos = 2;

    for (int i = 0; i < 3; ++i) {
        Tsc tsc("depth2_key2");
        (void)ai.thinkPlan(frameId, cf, kumipuyoSeq, depth, numKeyPuyos);
    }

    double average, variance;
    Tsc::GetStatistics("depth2_key2", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

