#include "mayah_ai.h"

#include <gtest/gtest.h>
#include "core/algorithm/puyo_possibility.h"
#include "core/kumipuyo.h"

using namespace std;

TEST(MayahAITest, DontCrash)
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

    Gazer gazer;
    gazer.initializeWith(1);

    (void)MayahAI().think(1, f, seq);
}

