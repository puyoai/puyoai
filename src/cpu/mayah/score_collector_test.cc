#include "score_collector.h"

#include <gtest/gtest.h>

using namespace std;

TEST(ScoreCollectorTest, score)
{
    EvaluationParameterMap m;
    m.mutableMoveParamSet()->setDefault(TOTAL_FRAMES, 3.0);
    m.mutableMoveParamSet()->setParam(EvaluationMode::EARLY, TOTAL_FRAMES, 5.0);

    SimpleScoreCollector collector(m);
    collector.addScore(TOTAL_FRAMES, 10.0);

    EXPECT_EQ(50.0, collector.collectedScore().score(EvaluationMode::EARLY));
    EXPECT_EQ(30.0, collector.collectedScore().score(EvaluationMode::MIDDLE));
}
