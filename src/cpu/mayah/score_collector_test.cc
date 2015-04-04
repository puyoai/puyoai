#include "score_collector.h"

#include <gtest/gtest.h>

using namespace std;

TEST(ScoreCollectorTest, score)
{
    EvaluationParameterMap m;
    m.mutableParameter(EvaluationMode::DEFAULT)->setValue(SCORE, 3.0);
    m.mutableParameter(EvaluationMode::EARLY)->setValue(SCORE, 5.0);

    SimpleScoreCollector collector(m);
    collector.addScore(SCORE, 10.0);

    EXPECT_EQ(30.0, collector.collectedScore().score(EvaluationMode::DEFAULT));
    EXPECT_EQ(50.0, collector.collectedScore().score(EvaluationMode::EARLY));
    EXPECT_EQ(30.0, collector.collectedScore().score(EvaluationMode::MIDDLE));
}
