#include "puyo_possibility.h"

#include <gtest/gtest.h>

#include "core/kumipuyo_seq.h"

TEST(PuyoPossibilityTest, possibility)
{
    PuyoPossibility::initialize();

    EXPECT_EQ(1.0, PuyoPossibility::possibility(PuyoSet(0, 0, 0, 0), 0));

    EXPECT_EQ(4.0 / 4.0, PuyoPossibility::possibility(PuyoSet(0, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoPossibility::possibility(PuyoSet(1, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoPossibility::possibility(PuyoSet(0, 1, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoPossibility::possibility(PuyoSet(0, 0, 1, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoPossibility::possibility(PuyoSet(0, 0, 0, 1), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoPossibility::possibility(PuyoSet(1, 1, 0, 0), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoPossibility::possibility(PuyoSet(0, 1, 1, 0), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoPossibility::possibility(PuyoSet(0, 0, 1, 1), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoPossibility::possibility(PuyoSet(1, 0, 0, 1), 1));

    EXPECT_EQ(16.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 0, 0, 0), 2));

    EXPECT_EQ(7.0 / 16.0, PuyoPossibility::possibility(PuyoSet(1, 0, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 1, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 0, 1, 0), 2));
    EXPECT_EQ(7.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 0, 0, 1), 2));

    EXPECT_EQ(1.0 / 16.0, PuyoPossibility::possibility(PuyoSet(2, 0, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 2, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 0, 2, 0), 2));
    EXPECT_EQ(1.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 0, 0, 2), 2));

    EXPECT_EQ(2.0 / 16.0, PuyoPossibility::possibility(PuyoSet(1, 1, 0, 0), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoPossibility::possibility(PuyoSet(1, 0, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoPossibility::possibility(PuyoSet(1, 0, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 1, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 1, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoPossibility::possibility(PuyoSet(0, 0, 1, 1), 2));

    // (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 256.0, PuyoPossibility::possibility(PuyoSet(2, 1, 1, 1), 5));

    // RGBY-R or RGBY-G or RGBY-B or RGBY-Y
    // 4 * (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 64.0, PuyoPossibility::possibility(PuyoSet(1, 1, 1, 1), 5));
}

TEST(PuyoPossibilityTest, necessaryPuyos)
{
    PuyoPossibility::initialize();

    EXPECT_EQ(0, PuyoPossibility::necessaryPuyos(PuyoSet(0, 0, 0, 0), 1.0));
    EXPECT_EQ(0, PuyoPossibility::necessaryPuyos(PuyoSet(0, 0, 0, 0), 0.8));

    EXPECT_EQ(1, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.249));
    EXPECT_EQ(1, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.25));
    EXPECT_EQ(2, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.251));

    EXPECT_EQ(1, PuyoPossibility::necessaryPuyos(PuyoSet(0, 1, 0, 0), 0.25));
    EXPECT_EQ(1, PuyoPossibility::necessaryPuyos(PuyoSet(0, 0, 1, 0), 0.25));
    EXPECT_EQ(1, PuyoPossibility::necessaryPuyos(PuyoSet(0, 0, 0, 1), 0.25));

    EXPECT_EQ(1, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.2));
    EXPECT_EQ(4, PuyoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.2));
    EXPECT_EQ(7, PuyoPossibility::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.2));
    EXPECT_EQ(3, PuyoPossibility::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.2));
    EXPECT_EQ(7, PuyoPossibility::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.2));

    EXPECT_EQ(3, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.5));
    EXPECT_EQ(7, PuyoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.5));
    EXPECT_EQ(11, PuyoPossibility::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.5));
    EXPECT_EQ(5, PuyoPossibility::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.5));
    EXPECT_EQ(10, PuyoPossibility::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.5));

    EXPECT_EQ(6, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.8));
    EXPECT_EQ(11, PuyoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.8));
    EXPECT_EQ(16, PuyoPossibility::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.8));
    EXPECT_EQ(8, PuyoPossibility::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.8));
    EXPECT_EQ(14, PuyoPossibility::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.8));
}

TEST(PuyoPossibilityTest, necessaryPuyosWithKumipuyoSeq)
{
    PuyoPossibility::initialize();

    EXPECT_EQ(0, PuyoPossibility::necessaryPuyos(PuyoSet(0, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(2, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), KumipuyoSeq("RR"), 0.25));
    EXPECT_EQ(2, PuyoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(2, PuyoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(9, PuyoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), KumipuyoSeq("GG"), 0.5));
}
