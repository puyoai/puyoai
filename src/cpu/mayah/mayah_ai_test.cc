#include "mayah_ai.h"

#include <gtest/gtest.h>
#include "core/algorithm/puyo_possibility.h"
#include "core/kumipuyo_seq.h"

using namespace std;

unique_ptr<MayahAI> makeAI()
{
    int argc = 1;
    char arg[] = "mayah";
    char* argv[] = {arg};
    return unique_ptr<MayahAI>(new MayahAI(argc, argv));
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
    ai->initializeGazerForTest(1);
    (void)ai->think(1, f, seq, AdditionalThoughtInfo());
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
    ai->initializeGazerForTest(1);
    (void)ai->think(1, f, seq, AdditionalThoughtInfo());
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
    ai->initializeGazerForTest(1);
    (void)ai->think(1, f, seq, AdditionalThoughtInfo());
}
