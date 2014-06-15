#include "gazer.h"

#include <gtest/gtest.h>
#include "core/algorithm/puyo_possibility.h"
#include "core/kumipuyo.h"

using namespace std;

TEST(GazerTest, UpdatePossibleRensasTest)
{
    TsumoPossibility::initialize();

    Gazer gazer;
    gazer.initializeWith(100);

    CoreField f("000000"
                "000000"
                "000000"
                "456000"
                "445664"
                "557774");

    KumipuyoSeq kumipuyos("777777");
    gazer.updatePossibleRensas(f, kumipuyos);

    EXPECT_EQ(gazer.possibleRensaInfos().size(), 3U);
    EXPECT_EQ(gazer.possibleRensaInfos().back().chains, 4);
}

TEST(GazerTest, UpdatePossibleRensasTest2)
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
