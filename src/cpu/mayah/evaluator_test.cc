#include "evaluator.h"

#include <algorithm>
#include <gtest/gtest.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "gazer.h"

using namespace std;

class EvaluatorTest : public testing::Test {
protected:
    CollectedFeatureScore eval(const CoreField& f, int numIteration = 1)
    {
        PuyoPossibility::initialize();

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
        evaluator.eval(plan, KumipuyoSeq(), 1, numIteration, PlayerState(), PlayerState(), preEvalResult, MidEvalResult(), false, gazer.gazeResult());
        return sc.collectedScore();
    }

    template<typename F>
    CollectedFeatureScore withEvaluator(F f) {
        PuyoPossibility::initialize();

        EvaluationParameterMap evaluationParameterMap;
        PatternBook patternBook;
        FeatureScoreCollector sc(evaluationParameterMap);
        Evaluator<FeatureScoreCollector> evaluator(patternBook, &sc);

        f(&evaluator);

        return sc.collectedScore();
    }

    template<typename F>
    CollectedFeatureScore withRensaEvaluator(F f) {
        PuyoPossibility::initialize();

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
    FeatureRensaScoreCollector sc(paramSet, paramSet);
    RensaEvaluator<FeatureRensaScoreCollector> evaluator(patternBook, &sc);

    evaluator.evalRensaGarbage(f);
    CollectedFeatureRensaScore cfs = sc.mainRensaScore();

    EXPECT_EQ(10, cfs.feature(NUM_GARBAGE_PUYOS));
    EXPECT_EQ(6, cfs.feature(NUM_SIDE_GARBAGE_PUYOS));
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
