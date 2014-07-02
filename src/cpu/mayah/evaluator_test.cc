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
    CollectedFeature eval(const CoreField& f)
    {
        TsumoPossibility::initialize();

        FeatureParameter parameter;
        Evaluator evaluator(parameter);
        Gazer gazer;

        gazer.initializeWith(1);

        vector<Decision> decisions { Decision(3, 0) };
        RensaResult rensaResult;
        int initiatingFrames = 10;

        RefPlan plan(f, decisions, rensaResult, 0, initiatingFrames);

        return evaluator.evalWithCollectingFeature(plan, f, 1, false, gazer);
    }
};

TEST_F(EvaluatorTest, RidgeHeight1)
{
    CoreField f(
        "  O   "
        "  O   "
        "  O   "
        "  O  O"
        "OOOOOO");

    CollectedFeature cf = eval(f);
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

    CollectedFeature cf = eval(f);
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

    CollectedFeature cf = eval(f);
    const vector<int>& vs = cf.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 1) != vs.end());
}

TEST_F(EvaluatorTest, ConnectionHorizontal1)
{
    CoreField f(
        "B B B "
        "GG YY "
        "RRRGGG");
    CollectedFeature cf = eval(f);

    map<int, int> m;
    for (auto v : cf.feature(CONNECTION_HORIZONTAL))
        m[v]++;

    EXPECT_EQ(2, m[3]);
    EXPECT_EQ(2, m[2]);
    EXPECT_EQ(3, m[1]);
    EXPECT_EQ(0, m[0]);
}

TEST_F(EvaluatorTest, ConnectionHorizontal2)
{
    CoreField f(
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");
    CollectedFeature cf = eval(f);

    EXPECT_TRUE(cf.feature(CONNECTION_HORIZONTAL).empty());
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
