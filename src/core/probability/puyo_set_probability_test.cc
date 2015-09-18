#include "core/probability/puyo_set_probability.h"

#include <gtest/gtest.h>

#include "core/kumipuyo_seq.h"

using namespace std;

TEST(PuyoSetProbabilityTest, possibility)
{
    PuyoSetProbability::initialize();

    EXPECT_EQ(1.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 0, 0), 0));

    EXPECT_EQ(4.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(1, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(0, 1, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 1, 0), 1));
    EXPECT_EQ(1.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 0, 1), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(1, 1, 0, 0), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(0, 1, 1, 0), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 1, 1), 1));
    EXPECT_EQ(0.0 / 4.0, PuyoSetProbability::possibility(PuyoSet(1, 0, 0, 1), 1));

    EXPECT_EQ(16.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 0, 0), 2));

    EXPECT_EQ(7.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(1, 0, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 1, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 1, 0), 2));
    EXPECT_EQ(7.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 0, 1), 2));

    EXPECT_EQ(1.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(2, 0, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 2, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 2, 0), 2));
    EXPECT_EQ(1.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 0, 2), 2));

    EXPECT_EQ(2.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(1, 1, 0, 0), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(1, 0, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(1, 0, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 1, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 1, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, PuyoSetProbability::possibility(PuyoSet(0, 0, 1, 1), 2));

    // (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 256.0, PuyoSetProbability::possibility(PuyoSet(2, 1, 1, 1), 5));

    // RGBY-R or RGBY-G or RGBY-B or RGBY-Y
    // 4 * (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 64.0, PuyoSetProbability::possibility(PuyoSet(1, 1, 1, 1), 5));
}

TEST(PuyoSetProbabilityTest, necessaryPuyos)
{
    PuyoSetProbability::initialize();

    EXPECT_EQ(0, PuyoSetProbability::necessaryPuyos(PuyoSet(0, 0, 0, 0), 1.0));
    EXPECT_EQ(0, PuyoSetProbability::necessaryPuyos(PuyoSet(0, 0, 0, 0), 0.8));

    EXPECT_EQ(1, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.249));
    EXPECT_EQ(1, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.25));
    EXPECT_EQ(2, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.251));

    EXPECT_EQ(1, PuyoSetProbability::necessaryPuyos(PuyoSet(0, 1, 0, 0), 0.25));
    EXPECT_EQ(1, PuyoSetProbability::necessaryPuyos(PuyoSet(0, 0, 1, 0), 0.25));
    EXPECT_EQ(1, PuyoSetProbability::necessaryPuyos(PuyoSet(0, 0, 0, 1), 0.25));

    EXPECT_EQ(1, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.2));
    EXPECT_EQ(4, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.2));
    EXPECT_EQ(7, PuyoSetProbability::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.2));
    EXPECT_EQ(3, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.2));
    EXPECT_EQ(7, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.2));

    EXPECT_EQ(3, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.5));
    EXPECT_EQ(7, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.5));
    EXPECT_EQ(11, PuyoSetProbability::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.5));
    EXPECT_EQ(5, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.5));
    EXPECT_EQ(10, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.5));

    EXPECT_EQ(6, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.8));
    EXPECT_EQ(11, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.8));
    EXPECT_EQ(16, PuyoSetProbability::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.8));
    EXPECT_EQ(8, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.8));
    EXPECT_EQ(14, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.8));
}

TEST(PuyoSetProbabilityTest, necessaryPuyosWithKumipuyoSeq)
{
    PuyoSetProbability::initialize();

    EXPECT_EQ(0, PuyoSetProbability::necessaryPuyos(PuyoSet(0, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(2, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), KumipuyoSeq("RR"), 0.25));
    EXPECT_EQ(2, PuyoSetProbability::necessaryPuyos(PuyoSet(1, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(2, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(9, PuyoSetProbability::necessaryPuyos(PuyoSet(2, 0, 0, 0), KumipuyoSeq("GG"), 0.5));
}
