#include "gazer.h"

#include <gtest/gtest.h>
#include "core/algorithm/puyo_possibility.h"
#include "core/kumipuyo.h"

using namespace std;

TEST(GazerTest, dontCrash)
{
    // Should not crash in this test case.

    TsumoPossibility::initialize();

    Gazer gazer;
    gazer.initializeWith(100);

    CoreField f(" O    "
                " O O  " // 12
                "OO OOO"
                "OOOOOO"
                "OGOOOO"
                "OYOOOO" // 8
                "OOOOOO"
                "OOOOOO"
                "OOOOOO"
                "OOOOOO" // 4
                "OOOOOO"
                "OBOYOO"
                "BBOBBR");

    KumipuyoSeq kumipuyos("BBRBYB");
    gazer.updatePossibleRensas(f, kumipuyos);
}
