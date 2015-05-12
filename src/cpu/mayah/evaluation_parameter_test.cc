#include "evaluation_parameter.h"

#include <gtest/gtest.h>

using namespace std;

TEST(EvaluationParameterTest, toString)
{
    EvaluationMoveParameter param;
    param.setParam(TOTAL_FRAMES, 1.0);

    // Since only TOTAL_FRAMES parameter is changed, only it's be emitted.
    EXPECT_EQ("TOTAL_FRAMES = 1.000000\n", param.toString());
}

TEST(EvaluationParameterTest, setter)
{
    EvaluationParameterMap m;
    EXPECT_EQ(0.0, m.moveParamSet().param(EvaluationMode::EARLY, TOTAL_FRAMES));

    m.mutableMoveParamSet()->setDefault(TOTAL_FRAMES, 1.0);
    m.mutableMoveParamSet()->setParam(EvaluationMode::EARLY, TOTAL_FRAMES, 2.0);

    EXPECT_EQ(2.0, m.moveParamSet().param(EvaluationMode::EARLY, TOTAL_FRAMES));
    EXPECT_EQ(1.0, m.moveParamSet().param(EvaluationMode::MIDDLE, TOTAL_FRAMES));
}
