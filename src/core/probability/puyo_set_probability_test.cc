#include "core/probability/puyo_set_probability.h"

#include <gtest/gtest.h>

#include "core/kumipuyo_seq.h"

using namespace std;

TEST(PuyoSetProbabilityTest, possibility)
{
    const PuyoSetProbability* prob = PuyoSetProbability::instanceSlow();

    EXPECT_EQ(1.0, prob->possibility(PuyoSet(0, 0, 0, 0), 0));

    EXPECT_EQ(4.0 / 4.0, prob->possibility(PuyoSet(0, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, prob->possibility(PuyoSet(1, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, prob->possibility(PuyoSet(0, 1, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, prob->possibility(PuyoSet(0, 0, 1, 0), 1));
    EXPECT_EQ(1.0 / 4.0, prob->possibility(PuyoSet(0, 0, 0, 1), 1));
    EXPECT_EQ(0.0 / 4.0, prob->possibility(PuyoSet(1, 1, 0, 0), 1));
    EXPECT_EQ(0.0 / 4.0, prob->possibility(PuyoSet(0, 1, 1, 0), 1));
    EXPECT_EQ(0.0 / 4.0, prob->possibility(PuyoSet(0, 0, 1, 1), 1));
    EXPECT_EQ(0.0 / 4.0, prob->possibility(PuyoSet(1, 0, 0, 1), 1));

    EXPECT_EQ(16.0 / 16.0, prob->possibility(PuyoSet(0, 0, 0, 0), 2));

    EXPECT_EQ(7.0 / 16.0, prob->possibility(PuyoSet(1, 0, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, prob->possibility(PuyoSet(0, 1, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, prob->possibility(PuyoSet(0, 0, 1, 0), 2));
    EXPECT_EQ(7.0 / 16.0, prob->possibility(PuyoSet(0, 0, 0, 1), 2));

    EXPECT_EQ(1.0 / 16.0, prob->possibility(PuyoSet(2, 0, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, prob->possibility(PuyoSet(0, 2, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, prob->possibility(PuyoSet(0, 0, 2, 0), 2));
    EXPECT_EQ(1.0 / 16.0, prob->possibility(PuyoSet(0, 0, 0, 2), 2));

    EXPECT_EQ(2.0 / 16.0, prob->possibility(PuyoSet(1, 1, 0, 0), 2));
    EXPECT_EQ(2.0 / 16.0, prob->possibility(PuyoSet(1, 0, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, prob->possibility(PuyoSet(1, 0, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, prob->possibility(PuyoSet(0, 1, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, prob->possibility(PuyoSet(0, 1, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, prob->possibility(PuyoSet(0, 0, 1, 1), 2));

    // (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 256.0, prob->possibility(PuyoSet(2, 1, 1, 1), 5));

    // RGBY-R or RGBY-G or RGBY-B or RGBY-Y
    // 4 * (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 64.0, prob->possibility(PuyoSet(1, 1, 1, 1), 5));
}

TEST(PuyoSetProbabilityTest, necessaryPuyos)
{
    const PuyoSetProbability* prob = PuyoSetProbability::instanceSlow();

    EXPECT_EQ(0, prob->necessaryPuyos(PuyoSet(0, 0, 0, 0), 1.0));
    EXPECT_EQ(0, prob->necessaryPuyos(PuyoSet(0, 0, 0, 0), 0.8));

    EXPECT_EQ(1, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.249));
    EXPECT_EQ(1, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.25));
    EXPECT_EQ(2, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.251));

    EXPECT_EQ(1, prob->necessaryPuyos(PuyoSet(0, 1, 0, 0), 0.25));
    EXPECT_EQ(1, prob->necessaryPuyos(PuyoSet(0, 0, 1, 0), 0.25));
    EXPECT_EQ(1, prob->necessaryPuyos(PuyoSet(0, 0, 0, 1), 0.25));

    EXPECT_EQ(1, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.2));
    EXPECT_EQ(4, prob->necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.2));
    EXPECT_EQ(7, prob->necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.2));
    EXPECT_EQ(3, prob->necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.2));
    EXPECT_EQ(7, prob->necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.2));

    EXPECT_EQ(3, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.5));
    EXPECT_EQ(7, prob->necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.5));
    EXPECT_EQ(11, prob->necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.5));
    EXPECT_EQ(5, prob->necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.5));
    EXPECT_EQ(10, prob->necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.5));

    EXPECT_EQ(6, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.8));
    EXPECT_EQ(11, prob->necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.8));
    EXPECT_EQ(16, prob->necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.8));
    EXPECT_EQ(8, prob->necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.8));
    EXPECT_EQ(14, prob->necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.8));
}

TEST(PuyoSetProbabilityTest, necessaryPuyosWithKumipuyoSeq)
{
    const PuyoSetProbability* prob = PuyoSetProbability::instanceSlow();

    EXPECT_EQ(0, prob->necessaryPuyos(PuyoSet(0, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(2, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), KumipuyoSeq("RR"), 0.25));
    EXPECT_EQ(2, prob->necessaryPuyos(PuyoSet(1, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(2, prob->necessaryPuyos(PuyoSet(2, 0, 0, 0), KumipuyoSeq("RR"), 1.0));

    EXPECT_EQ(9, prob->necessaryPuyos(PuyoSet(2, 0, 0, 0), KumipuyoSeq("GG"), 0.5));
}
