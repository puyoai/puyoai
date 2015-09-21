#include "mayah_ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "base/executor.h"
#include "core/frame_request.h"
#include "core/kumipuyo_seq.h"
#include "core/probability/puyo_set_probability.h"

using namespace std;

static unique_ptr<DebuggableMayahAI> makeAI(Executor* executor = nullptr)
{
    int argc = 1;
    char arg[] = "mayah";
    char* argv[] = {arg};
    return unique_ptr<DebuggableMayahAI>(new DebuggableMayahAI(argc, argv, executor));
}

// Parallel execution should return the same result as single thread execution.
TEST(MayahAITest, parallel)
{
    CoreField f(
        " R    "
        "YY BBB"
        "RRRGGG");

    unique_ptr<Executor> executor(Executor::makeDefaultExecutor());
    KumipuyoSeq seq("GGRRBY");

    auto ai = makeAI();
    auto parallelAi = makeAI(executor.get());

    ThoughtResult thoughtResult = ai->thinkPlan(2, f, seq, PlayerState(), PlayerState(), 2, 3);
    ThoughtResult parallelThoughtResult = parallelAi->thinkPlan(2, f, seq, PlayerState(), PlayerState(), 2, 3);

    EXPECT_EQ(thoughtResult.plan, parallelThoughtResult.plan);
    EXPECT_EQ(thoughtResult.rensaScore, parallelThoughtResult.rensaScore);
    EXPECT_EQ(thoughtResult.virtualRensaScore, parallelThoughtResult.virtualRensaScore);
}

// TODO(mayah): Move this test to situation_test.
TEST(MayahAITest, fromReal1)
{
    CoreField myField(
        "@     " // 12
        "@    G"
        "@    G"
        "Y G  R"
        "YGY GG" // 8
        "BRRRBY"
        "G@@YRY"
        "G@@BRY"
        "GBYBYG" // 4
        "BYRBGG"
        "BYYRBB"
        "RRRBRG");

    CoreField enemyField(
        "RG    "
        "GY    "
        "GG    "
        "YR    "
        "GRG   "
        "GRB   "
        "RGY  B"
        "RBR YR"
        "BRG RB"
        "RRYYRR");

    KumipuyoSeq mySeq("RYBR");
    KumipuyoSeq enemySeq("YRGRBG");

    auto ai = makeAI();
    FrameRequest req;
    req.frameId = 1;
    ai->gameWillBegin(req);

    ai->gaze(1148, enemyField, enemySeq);

    PlayerState me;
    me.pendingOjama = 67;

    PlayerState enemy;
    enemy.currentChain = 5;
    enemy.currentChainStartedFrameId = 1000;
    enemy.currentRensaResult = RensaResult(5, 10000, 346, false);

    ThoughtResult thoughtResult = ai->thinkPlan(1328, myField, mySeq, me, enemy, MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);

    EXPECT_TRUE(thoughtResult.plan.isRensaPlan());
}

TEST(MayahAITest, fromReal2)
{
    CoreField myField(
        "@     " // 12
        "@    G"
        "@G   G"
        "YYG  R"
        "YGY GG" // 8
        "BRRRBY"
        "G@@YRY"
        "G@@BRY"
        "GBYBYG" // 4
        "BYRBGG"
        "BYYRBB"
        "RRRBRG");

    CoreField enemyField(
        "RG    "
        "GY    "
        "GG    "
        "YR    "
        "GRG   "
        "GRB   "
        "RGY  B"
        "RBR YR"
        "BRG RB"
        "RRYYRR");

    KumipuyoSeq mySeq("BRYG");
    KumipuyoSeq enemySeq("YRGRBG");

    auto ai = makeAI();
    FrameRequest req;
    req.frameId = 1;
    ai->gameWillBegin(req);

    ai->gaze(1148, enemyField, enemySeq);

    PlayerState me;
    me.pendingOjama = 67;

    PlayerState enemy;
    enemy.currentChain = 5;
    enemy.currentChainStartedFrameId = 1000;
    enemy.currentRensaResult = RensaResult(5, 10000, 346, false);

    ThoughtResult thoughtResult = ai->thinkPlan(1352, myField, mySeq, me, enemy, MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);

    EXPECT_TRUE(thoughtResult.plan.isRensaPlan());
}

TEST(MayahAITest, zenkeshi1)
{
    CoreField myField(
        ".GGG.."
        ".YYY..");

    CoreField enemyField;

    KumipuyoSeq mySeq("GYRB");
    KumipuyoSeq enemySeq("GYBR");

    auto ai = makeAI();
    FrameRequest req;
    req.frameId = 1;
    ai->gameWillBegin(req);

    PlayerState me;
    PlayerState enemy;

    ThoughtResult thoughtResult = ai->thinkPlan(10, myField, mySeq, me, enemy, MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);

    EXPECT_TRUE(thoughtResult.plan.firstDecision() == Decision(3, 0) || thoughtResult.plan.firstDecision() == Decision(3, 3));
}

TEST(MayahAITest, DontCrash1)
{
    CoreField f(
        "OO  OO"
        "OO  OO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOYG"
        "OOOOOO"
        "OOOOOO"
        "BGOOOO"
        "YYOOOO"
        "BGOOOO"
        "GGOORY"
        "BBBRRB");
    KumipuyoSeq seq("GGRG");

    auto ai = makeAI();
    FrameRequest req;
    req.frameId = 1;
    ai->gameWillBegin(req);

    PlayerState me;
    me.field = f;
    me.seq = seq;

    PlayerState enemy;

    (void)ai->think(100, f, seq, me, enemy, false);
}

TEST(MayahAITest, DontCrash2)
{
    CoreField f(
        "G     "
        "RR    "
        "BG    "
        "RRGR  "
        "BRYB  "
        "BBYY  "
        "GRBBG "
        "GRRBGB"
        "GYYGRB"
        "RBBGGR"
        "BYBYYR"
        "YYGGBR"
        "RRGYYB"
        "YRYGYB");
    KumipuyoSeq seq("YYRG");

    auto ai = makeAI();
    FrameRequest req;
    req.frameId = 1;
    ai->gameWillBegin(req);

    PlayerState me;
    me.field = f;
    me.seq = seq;

    PlayerState enemy;

    (void)ai->think(100, f, seq, me, enemy, false);
}

TEST(MayahAITest, DontCrash3)
{
    CoreField f(
        "OO OOO"
        "OO OOO"
        "OO OOO"
        "OBOOOO"
        "RYOOOY"
        "BGOOBR"
        "GROBBG"
        "YGORGR"
        "YYGGRR"
        "GGBYGG"
        "GYYBBG"
        "RRRYYB"
        "BGYGBB");

    KumipuyoSeq seq("RYRR");

    auto ai = makeAI();
    FrameRequest req;
    req.frameId = 1;
    ai->gameWillBegin(req);

    PlayerState me;
    me.field = f;
    me.seq = seq;

    PlayerState enemy;

    (void)ai->think(100, f, seq, me, enemy, false);
}

#if 0
TEST(MayahAITest, setEvaluationParameter)
{
    auto ai = makeAI();
    EXPECT_EQ(0, ai->evaluationParameter(EvaluationMode::DEFAULT).getValue(SCORE));

    EvaluationParameterMap map = ai->evaluationParameterMap();
    map.mutableDefaultParameter()->setValue(SCORE, 1.0);
    ai->setEvaluationParameterMap(map);

    EXPECT_EQ(1.0, ai->evaluationParameter(EvaluationMode::DEFAULT).getValue(SCORE));
    EXPECT_EQ(1.0, ai->evaluationParameter(EvaluationMode::EARLY).getValue(SCORE));
}
#endif

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    google::ParseCommandLineFlags(&argc, &argv, true);

    return RUN_ALL_TESTS();
}
