#include "mayah_ai.h"

#include <gtest/gtest.h>

#include "base/executor.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/frame_request.h"
#include "core/kumipuyo_seq.h"

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
    TsumoPossibility::initialize();

    CoreField f(
        " R    "
        "YY BBB"
        "RRRGGG");

    unique_ptr<Executor> executor(Executor::makeDefaultExecutor());
    KumipuyoSeq seq("GGRRBY");

    auto ai = makeAI();
    auto parallelAi = makeAI(executor.get());

    ThoughtResult thoughtResult = ai->thinkPlan(2, f, seq, AdditionalThoughtInfo(), 2, 3);
    ThoughtResult parallelThoughtResult = parallelAi->thinkPlan(2, f, seq, AdditionalThoughtInfo(), 2, 3);

    EXPECT_EQ(thoughtResult.plan, parallelThoughtResult.plan);
    EXPECT_EQ(thoughtResult.isRensaPlan, parallelThoughtResult.isRensaPlan);
    EXPECT_EQ(thoughtResult.rensaScore, parallelThoughtResult.rensaScore);
    EXPECT_EQ(thoughtResult.virtualRensaScore, parallelThoughtResult.virtualRensaScore);
}

TEST(MayahAITest, DontCrash1)
{
    TsumoPossibility::initialize();

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
    (void)ai->think(100, f, seq, AdditionalThoughtInfo());
}

TEST(MayahAITest, DontCrash2)
{
    TsumoPossibility::initialize();

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
    (void)ai->think(100, f, seq, AdditionalThoughtInfo());
}

TEST(MayahAITest, DontCrash3)
{
    TsumoPossibility::initialize();

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
    (void)ai->think(100, f, seq, AdditionalThoughtInfo());
}
