#include "enemy_info.h"

#include <gtest/gtest.h>
#include "core/algorithm/puyo_possibility.h"
#include "core/kumipuyo.h"

using namespace std;

TEST(EnemyInfoTest, UpdatePossibleRensasTest)
{
    TsumoPossibility::initialize();

    EnemyInfo info;
    info.initializeWith(100);

    CoreField f("000000"
                "000000"
                "000000"
                "456000"
                "445664"
                "557774");

    KumipuyoSeq kumipuyos("777777");
    info.updatePossibleRensas(f, kumipuyos);

    EXPECT_EQ(info.possibleRensaInfos().size(), 3U);
    EXPECT_EQ(info.possibleRensaInfos().back().chains, 4);
}

TEST(EnemyInfoTest, UpdatePossibleRensasTest2)
{
    // Should not crash in this test case.

    TsumoPossibility::initialize();

    EnemyInfo info;
    info.initializeWith(100);

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
    info.updatePossibleRensas(f, kumipuyos);
}
