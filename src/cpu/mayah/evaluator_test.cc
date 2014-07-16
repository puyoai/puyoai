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
    CollectedFeature eval(const CoreField& f, int numKeyPuyos = 0)
    {
        TsumoPossibility::initialize();

        FeatureParameter parameter;
        vector<BookField> books;
        Evaluator evaluator(parameter, books);
        Gazer gazer;

        gazer.initializeWith(1);

        vector<Decision> decisions { Decision(3, 0) };
        RensaResult rensaResult;
        int initiatingFrames = 10;
        int lastDropFrames = 10;

        RefPlan plan(f, decisions, rensaResult, 0, initiatingFrames, lastDropFrames);

        return evaluator.evalWithCollectingFeature(plan, f, 1, numKeyPuyos, gazer);
    }
};

TEST_F(EvaluatorTest, collectScoreForRensaGarbage)
{
    CoreField f("R    R"
                "R    R"
                "YYYGGG");

    FeatureParameter param;
    FeatureScoreCollector sc(param);

    collectScoreForRensaGarbage(&sc, f);
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

TEST_F(EvaluatorTest, connection)
{
    CoreField f("BBBYYY"
                "OOOOOO"
                "BBYYGO");

    FeatureParameter param;
    FeatureScoreCollector sc(param);

    collectScoreForConnection(&sc, f);
    CollectedFeature cf = sc.toCollectedFeature();

    map<int, int> vs;
    for (int v : cf.feature(CONNECTION)) {
        vs[v]++;
    }
    EXPECT_EQ(3U, vs.size());
    EXPECT_EQ(2, vs[3]);
    EXPECT_EQ(2, vs[2]);
    EXPECT_EQ(1, vs[1]);
}

TEST_F(EvaluatorTest, ConnectionHorizontal1)
{
    CoreField f(
        "B B B "
        "GG YY "
        "RRRGGG");
    CollectedFeature cf = eval(f);

    map<int, int> m[7];
    for (auto v : cf.feature(CONNECTION_HORIZONTAL_FROM_1))
        m[1][v]++;
    for (auto v : cf.feature(CONNECTION_HORIZONTAL_FROM_2))
        m[2][v]++;
    for (auto v : cf.feature(CONNECTION_HORIZONTAL_FROM_3))
        m[3][v]++;
    for (auto v : cf.feature(CONNECTION_HORIZONTAL_FROM_4))
        m[4][v]++;
    for (auto v : cf.feature(CONNECTION_HORIZONTAL_FROM_5))
        m[5][v]++;

    EXPECT_EQ(1, m[1][1]);
    EXPECT_EQ(1, m[1][2]);
    EXPECT_EQ(1, m[1][3]);

    EXPECT_EQ(0, m[2][1]);
    EXPECT_EQ(0, m[2][2]);
    EXPECT_EQ(0, m[2][3]);

    EXPECT_EQ(1, m[3][1]);
    EXPECT_EQ(0, m[3][2]);
    EXPECT_EQ(0, m[3][3]);

    EXPECT_EQ(0, m[4][1]);
    EXPECT_EQ(1, m[4][2]);
    EXPECT_EQ(1, m[4][3]);

    EXPECT_EQ(1, m[5][1]);
    EXPECT_EQ(0, m[5][2]);
    EXPECT_EQ(0, m[5][3]);
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
