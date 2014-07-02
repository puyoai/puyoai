#include "mayah_ai.h"

#include <gtest/gtest.h>
#include "core/algorithm/puyo_possibility.h"
#include "core/kumipuyo.h"

using namespace std;

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

    MayahAI ai;
    ai.initializeGazerForTest(1);
    (void)ai.think(1, f, seq);
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

    MayahAI ai;
    ai.initializeGazerForTest(1);
    (void)ai.think(1, f, seq);
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

    MayahAI ai;
    ai.initializeGazerForTest(1);
    (void)ai.think(1, f, seq);
}
