#include "core/algorithm/plan.h"

#include <iostream>
#include <gtest/gtest.h>

#include "base/tsc.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"

#include "mayah_ai.h"

using namespace std;

TEST(MayahAIPerformanceTest, THIRD)
{
    TsumoPossibility::initialize();

    MayahAI ai;
    int frameId = 1;
    CoreField cf;
    KumipuyoSeq kumipuyoSeq("RRGG");
    int depth = 3;
    int numKeyPuyos = 0;

    for (int i = 0; i < 3; ++i) {
        Tsc tsc("Depth3");
        ai.thinkPlan(frameId, cf, kumipuyoSeq, depth, numKeyPuyos);
    }

    double average, variance;
    Tsc::GetStatistics("Depth3", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

