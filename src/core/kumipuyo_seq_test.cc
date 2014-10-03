#include "core/kumipuyo_seq.h"

#include <gtest/gtest.h>

TEST(KumipuyoSeqTest, basic)
{
    KumipuyoSeq seq;
    EXPECT_EQ(0, seq.size());
}
