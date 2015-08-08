#include "core/kumipuyo_seq.h"

#include <gtest/gtest.h>

TEST(KumipuyoSeqTest, basic)
{
    KumipuyoSeq seq;
    EXPECT_EQ(0, seq.size());
}

TEST(KumipuyoSeqTest, axischild)
{
    KumipuyoSeq seq("RBYG");

    EXPECT_EQ(PuyoColor::RED, seq.axis(0));
    EXPECT_EQ(PuyoColor::BLUE, seq.child(0));
    EXPECT_EQ(PuyoColor::YELLOW, seq.axis(1));
    EXPECT_EQ(PuyoColor::GREEN, seq.child(1));

    EXPECT_EQ(2, seq.size());
}

TEST(KumipuyoSeqTest, color)
{
    KumipuyoSeq seq("RBYG");

    EXPECT_EQ(PuyoColor::RED, seq.color(NextPuyoPosition::CURRENT_AXIS));
    EXPECT_EQ(PuyoColor::BLUE, seq.color(NextPuyoPosition::CURRENT_CHILD));
    EXPECT_EQ(PuyoColor::YELLOW, seq.color(NextPuyoPosition::NEXT1_AXIS));
    EXPECT_EQ(PuyoColor::GREEN, seq.color(NextPuyoPosition::NEXT1_CHILD));
    EXPECT_EQ(PuyoColor::EMPTY, seq.color(NextPuyoPosition::NEXT2_AXIS));
    EXPECT_EQ(PuyoColor::EMPTY, seq.color(NextPuyoPosition::NEXT2_CHILD));
}
