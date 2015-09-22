#include "shape_evaluator.h"

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "core/plan/plan.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/probability/puyo_set_probability.h"

#include "evaluation_parameter.h"
#include "score_collector.h"

using namespace std;

class ShapeEvaluatorTest : public testing::Test {
protected:
    template<typename F>
    CollectedFeatureScore withEvaluator(F f) {
        EvaluationParameterMap evaluationParameterMap;
        FeatureScoreCollector sc(evaluationParameterMap);
        ShapeEvaluator<FeatureScoreCollector> evaluator(&sc);

        f(&evaluator);

        return sc.collectedScore();
    }
};

TEST_F(ShapeEvaluatorTest, RidgeHeight1)
{
    CoreField f(
        "  O   "
        "  O   "
        "  O   "
        "  O  O"
        "OOOOOO");

    CollectedFeatureScore cfs = withEvaluator([&f](ShapeEvaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cfs.moveScore.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 4) != vs.end());
}

TEST_F(ShapeEvaluatorTest, RidgeHeight2)
{
    CoreField f(
        "  O   "
        "  O   "
        "  OO  "
        " OOO O"
        "OOOOOO");

    CollectedFeatureScore cfs = withEvaluator([&f](ShapeEvaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cfs.moveScore.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 2) != vs.end());
}

TEST_F(ShapeEvaluatorTest, RidgeHeight3)
{
    CoreField f(
        "  O   "
        " OO   "
        " OO   "
        " OOO O"
        "OOOOOO");

    CollectedFeatureScore cfs = withEvaluator([&f](ShapeEvaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cfs.moveScore.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 1) != vs.end());
}

TEST_F(ShapeEvaluatorTest, connection)
{
    CoreField f("BBBYYY"
                "OOOOGO"
                "BBYYGO");

    CollectedFeatureScore cfs = withEvaluator([&f](ShapeEvaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalConnection(f);
    });

    EXPECT_EQ(2, cfs.moveScore.feature(CONNECTION_3));
    EXPECT_EQ(3, cfs.moveScore.feature(CONNECTION_2));
}

TEST_F(ShapeEvaluatorTest, connectionHorizontal)
{
    CoreField f(
        "OGGGOO"
        "OOYYOO"
        "RRROGG");

    CollectedFeatureScore cfs = withEvaluator([&f](ShapeEvaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRestrictedConnectionHorizontalFeature(f);
    });

    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_2));
    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_3));
    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_CROSSED_2));
    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_CROSSED_3));
}
