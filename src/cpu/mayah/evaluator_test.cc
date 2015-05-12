#include "evaluator.h"

#include <algorithm>
#include <gtest/gtest.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/decision.h"
#include "core/core_field.h"
#include "gazer.h"

using namespace std;

class EvaluatorTest : public testing::Test {
protected:
    CollectedFeatureScore eval(const CoreField& f, int numIteration = 1)
    {
        TsumoPossibility::initialize();

        EvaluationParameterMap evaluationParameterMap;
        PatternBook patternBook;
        Gazer gazer;

        gazer.initialize(100);

        vector<Decision> decisions { Decision(3, 0) };
        RensaResult rensaResult;
        int framesToIgnite = 10;
        int lastDropFrames = 10;

        RefPlan plan(f, decisions, rensaResult, 0, framesToIgnite, lastDropFrames, 0, 0, 0, 0, false);

        PreEvalResult preEvalResult = PreEvaluator(patternBook).preEval(f);
        FeatureScoreCollector sc(evaluationParameterMap);
        Evaluator<FeatureScoreCollector> evaluator(patternBook, &sc);
        evaluator.eval(plan, 1, numIteration, PlayerState(), PlayerState(), preEvalResult, MidEvalResult(), gazer.gazeResult());
        return sc.collectedScore();
    }

    template<typename F>
    CollectedFeatureScore withEvaluator(F f) {
        TsumoPossibility::initialize();

        EvaluationParameterMap evaluationParameterMap;
        PatternBook patternBook;
        FeatureScoreCollector sc(evaluationParameterMap);
        Evaluator<FeatureScoreCollector> evaluator(patternBook, &sc);

        f(&evaluator);

        return sc.collectedScore();
    }

    template<typename F>
    CollectedFeatureScore withRensaEvaluator(F f) {
        TsumoPossibility::initialize();

        EvaluationParameterMap evaluationParameterMap;
        PatternBook patternBook;
        FeatureScoreCollector sc(evaluationParameterMap);
        RensaEvaluator<FeatureScoreCollector> rensaEvaluator(patternBook, &sc);

        f(&rensaEvaluator);

        return sc.collectedScore();
    }
};

TEST_F(EvaluatorTest, evalRensaGarbage)
{
    CoreField f("R    R"
                "R    R"
                "YYYGGG");

    EvaluationRensaParameterSet paramSet;
    PatternBook patternBook;
    FeatureRensaScoreCollector sc(paramSet);
    RensaEvaluator<FeatureRensaScoreCollector> evaluator(patternBook, &sc);

    evaluator.evalRensaGarbage(f);
    CollectedFeatureRensaScore cfs = sc.collectedScore();

    EXPECT_EQ(10, cfs.feature(NUM_GARBAGE_PUYOS));
    EXPECT_EQ(6, cfs.feature(NUM_SIDE_GARBAGE_PUYOS));
}

TEST_F(EvaluatorTest, RidgeHeight1)
{
    CoreField f(
        "  O   "
        "  O   "
        "  O   "
        "  O  O"
        "OOOOOO");

    CollectedFeatureScore cfs = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cfs.moveScore.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 4) != vs.end());
}

TEST_F(EvaluatorTest, RidgeHeight2)
{
    CoreField f(
        "  O   "
        "  O   "
        "  OO  "
        " OOO O"
        "OOOOOO");

    CollectedFeatureScore cfs = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cfs.moveScore.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 2) != vs.end());
}

TEST_F(EvaluatorTest, RidgeHeight3)
{
    CoreField f(
        "  O   "
        " OO   "
        " OO   "
        " OOO O"
        "OOOOOO");

    CollectedFeatureScore cfs = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cfs.moveScore.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 1) != vs.end());
}

TEST_F(EvaluatorTest, connection)
{
    CoreField f("BBBYYY"
                "OOOOGO"
                "BBYYGO");

    CollectedFeatureScore cfs = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalConnection(f);
    });

    EXPECT_EQ(2, cfs.moveScore.feature(CONNECTION_3));
    EXPECT_EQ(3, cfs.moveScore.feature(CONNECTION_2));
}

TEST_F(EvaluatorTest, connectionHorizontal)
{
    CoreField f(
        "OGGGOO"
        "OOYYOO"
        "RRROGG");

    CollectedFeatureScore cfs = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRestrictedConnectionHorizontalFeature(f);
    });

    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_2));
    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_3));
    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_CROSSED_2));
    EXPECT_EQ(1, cfs.moveScore.feature(CONNECTION_HORIZONTAL_CROSSED_3));
}


TEST_F(EvaluatorTest, sideChain)
{
    CoreField f(
        ".R...."
        "RB...."
        "RB...."
        "RB...."
        "BR...."
        "BR...."
        "BRGYG."
        "RGYGB."
        "RGYGB."
        "RGYGB.");

    CollectedFeatureScore cfs = eval(f, 1);
    EXPECT_EQ(1.0, cfs.moveScore.feature(HOLDING_SIDE_CHAIN_MEDIUM));
}

TEST_F(EvaluatorTest, DontCrash1)
{
    CoreField f(
        "@@  @@"
        "@@  @@"
        "@@@@@@"
        "@@@@@@"
        "@@@@@@"
        "@@@@YG"
        "@@@@@@"
        "@@@@@@"
        "BG@@@@"
        "YY@@@@"
        "BG@@@@"
        "GG@@RY"
        "BBBRRB");
    (void)eval(f);
}

TEST_F(EvaluatorTest, DontCrash2)
{
    CoreField f;
    (void)eval(f);
}
