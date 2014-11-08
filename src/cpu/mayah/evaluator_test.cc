#include "evaluator.h"

#include <algorithm>
#include <gtest/gtest.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/decision.h"
#include "core/field/core_field.h"
#include "gazer.h"

using namespace std;

class EvaluatorTest : public testing::Test {
protected:
    CollectedFeature eval(const CoreField& f, int numIteration = 1)
    {
        TsumoPossibility::initialize();

        EvaluationParameter evaluationParameter;
        vector<BookField> books;
        Gazer gazer;

        gazer.initialize(100);

        vector<Decision> decisions { Decision(3, 0) };
        RensaResult rensaResult;
        int framesToInitiate = 10;
        int lastDropFrames = 10;

        RefPlan plan(f, decisions, rensaResult, 0, framesToInitiate, lastDropFrames);

        PreEvalResult preEvalResult = PreEvaluator(books).preEval(f);
        FeatureScoreCollector sc(evaluationParameter);
        Evaluator<FeatureScoreCollector> evaluator(books, &sc);
        evaluator.collectScore(plan, f, 1, numIteration, PlayerState(), PlayerState(), preEvalResult, MidEvalResult(), gazer.gazeResult());
        return sc.toCollectedFeature();
    }

    template<typename F>
    CollectedFeature withEvaluator(F f) {
        TsumoPossibility::initialize();

        EvaluationParameter evaluationParameter;
        vector<BookField> books;
        FeatureScoreCollector sc(evaluationParameter);
        Evaluator<FeatureScoreCollector> evaluator(books, &sc);

        f(&evaluator);

        return sc.toCollectedFeature();
    }
};

TEST_F(EvaluatorTest, collectScoreForRensaGarbage)
{
    CoreField f("R    R"
                "R    R"
                "YYYGGG");

    EvaluationParameter param;
    std::vector<BookField> books;
    FeatureScoreCollector sc(param);
    RensaEvaluator<FeatureScoreCollector> evaluator(books, &sc);

    evaluator.collectScoreForRensaGarbage(f);
    CollectedFeature cf = sc.toCollectedFeature();

    EXPECT_EQ(10, cf.feature(NUM_GARBAGE_PUYOS));
    EXPECT_EQ(6, cf.feature(NUM_SIDE_GARBAGE_PUYOS));
}

TEST_F(EvaluatorTest, RidgeHeight1)
{
    CoreField f(
        "  O   "
        "  O   "
        "  O   "
        "  O  O"
        "OOOOOO");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cf.feature(RIDGE_HEIGHT);
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

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cf.feature(RIDGE_HEIGHT);
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

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cf.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 1) != vs.end());
}

TEST_F(EvaluatorTest, connection)
{
    CoreField f("BBBYYY"
                "OOOOOO"
                "BBYYGO");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->collectScoreForConnection(f);
    });

    map<int, int> vs;
    for (int v : cf.feature(CONNECTION)) {
        vs[v]++;
    }
    EXPECT_EQ(3U, vs.size());
    EXPECT_EQ(2, vs[3]);
    EXPECT_EQ(2, vs[2]);
    EXPECT_EQ(1, vs[1]);
}

TEST_F(EvaluatorTest, connectionHorizontal)
{
    CoreField f(
        "OGGGOO"
        "OOYYOO"
        "RRROGG");

    CollectedFeature cf = eval(f);

    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_2));
    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_3));
    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_CROSSED_2));
    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_CROSSED_3));
}

TEST_F(EvaluatorTest, NumUnreachableSpace1)
{
    CoreField f(
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");
    CollectedFeature cf = eval(f);

    EXPECT_EQ(0, cf.feature(NUM_UNREACHABLE_SPACE));
}

TEST_F(EvaluatorTest, NumUnreachableSpace2)
{
    CoreField f(
        "    O " // 12
        "    O "
        "    O "
        "    O "
        "    O " // 8
        "    O "
        "    O "
        "    O "
        "    O " // 4
        "    O "
        "    O "
        "    O ");
    CollectedFeature cf = eval(f);

    EXPECT_EQ(12, cf.feature(NUM_UNREACHABLE_SPACE));
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
