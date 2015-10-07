#include "evaluator.h"

#include <algorithm>
#include <gtest/gtest.h>

#include "core/plan/plan.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/probability/puyo_set_probability.h"
#include "gazer.h"

using namespace std;

class EvaluatorTest : public testing::Test {
protected:
    CollectedFeatureScore eval(const CoreField& f, int numIteration = 1)
    {
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
        evaluator.eval(plan, KumipuyoSeq(), 1, numIteration, PlayerState(), PlayerState(), preEvalResult, MidEvalResult(), false, false, gazer.gazeResult());
        return sc.collectedScore();
    }

    template<typename F>
    CollectedFeatureScore withEvaluator(F f) {
        EvaluationParameterMap evaluationParameterMap;
        PatternBook patternBook;
        FeatureScoreCollector sc(evaluationParameterMap);
        Evaluator<FeatureScoreCollector> evaluator(patternBook, &sc);

        f(&evaluator);

        return sc.collectedScore();
    }

    template<typename F>
    CollectedFeatureScore withRensaEvaluator(F f) {
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

TEST_F(EvaluatorTest, evalFirePointTabooFeature1)
{
    CoreField cf("RR.R..");
    FieldBits ignitionBits(
        "1111..");

    EvaluationRensaParameterSet paramSet;
    PatternBook patternBook;
    FeatureRensaScoreCollector sc(paramSet, paramSet);
    RensaEvaluator<FeatureRensaScoreCollector> evaluator(patternBook, &sc);

    evaluator.evalFirePointTabooFeature(cf, ignitionBits);
    CollectedFeatureRensaScore cfs = sc.mainRensaScore();

    EXPECT_EQ(1, cfs.feature(FIRE_POINT_TABOO));
}

TEST_F(EvaluatorTest, evalFirePointTabooFeature2)
{
    CoreField cf(
        "R....."
        "R.R...");
    FieldBits ignitionBits(
        "1....."
        "111...");

    EvaluationRensaParameterSet paramSet;
    PatternBook patternBook;
    FeatureRensaScoreCollector sc(paramSet, paramSet);
    RensaEvaluator<FeatureRensaScoreCollector> evaluator(patternBook, &sc);

    evaluator.evalFirePointTabooFeature(cf, ignitionBits);
    CollectedFeatureRensaScore cfs = sc.mainRensaScore();

    EXPECT_EQ(1, cfs.feature(FIRE_POINT_TABOO));
}

TEST_F(EvaluatorTest, evalFirePointTabooFeature3)
{
    CoreField cf(
        "..R..."
        "R.R...");
    FieldBits ignitionBits(
        "..1..."
        "111...");

    EvaluationRensaParameterSet paramSet;
    PatternBook patternBook;
    FeatureRensaScoreCollector sc(paramSet, paramSet);
    RensaEvaluator<FeatureRensaScoreCollector> evaluator(patternBook, &sc);

    evaluator.evalFirePointTabooFeature(cf, ignitionBits);
    CollectedFeatureRensaScore cfs = sc.mainRensaScore();

    EXPECT_EQ(1, cfs.feature(FIRE_POINT_TABOO));
}

TEST_F(EvaluatorTest, evalFirePointTabooFeature4)
{
    CoreField cf(
        "...R.."
        ".R.R..");
    FieldBits ignitionBits(
        "...1.."
        ".111..");

    EvaluationRensaParameterSet paramSet;
    PatternBook patternBook;
    FeatureRensaScoreCollector sc(paramSet, paramSet);
    RensaEvaluator<FeatureRensaScoreCollector> evaluator(patternBook, &sc);

    evaluator.evalFirePointTabooFeature(cf, ignitionBits);
    CollectedFeatureRensaScore cfs = sc.mainRensaScore();

    EXPECT_EQ(1, cfs.feature(FIRE_POINT_TABOO));
}

TEST_F(EvaluatorTest, evalFirePointTabooFeature5)
{
    CoreField cf(
        "......"
        "R.R...");
    FieldBits ignitionBits(
        "..1..."
        "111...");

    EvaluationRensaParameterSet paramSet;
    PatternBook patternBook;
    FeatureRensaScoreCollector sc(paramSet, paramSet);
    RensaEvaluator<FeatureRensaScoreCollector> evaluator(patternBook, &sc);

    evaluator.evalFirePointTabooFeature(cf, ignitionBits);
    CollectedFeatureRensaScore cfs = sc.mainRensaScore();

    EXPECT_EQ(0, cfs.feature(FIRE_POINT_TABOO));
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
