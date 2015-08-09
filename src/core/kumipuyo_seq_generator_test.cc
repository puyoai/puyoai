#include "core/kumipuyo_seq_generator.h"

#include <map>

#include <gtest/gtest.h>

#include "core/puyo_color.h"

using namespace std;

TEST(KumipuyoSeqGeneratorTest, generateRandomSequence)
{
    KumipuyoSeq seq = KumipuyoSeqGenerator::generateRandomSequence(10);

    EXPECT_EQ(10, seq.size());
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(isNormalColor(seq.axis(i)));
        EXPECT_TRUE(isNormalColor(seq.child(i)));
    }
}

TEST(KumipuyoSeqGeneratorTest, checkRestriction)
{
    KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();

    EXPECT_EQ(128, seq.size());

    // The first 3 hand should be RED, BLUE or YELLOW.
    for (int i = 0; i < 3; ++i) {
        EXPECT_NE(PuyoColor::GREEN, seq.axis(i));
        EXPECT_NE(PuyoColor::GREEN, seq.child(i));
    }

    map<PuyoColor, int> count;
    for (int i = 0; i < seq.size(); ++i) {
        count[seq.axis(i)]++;
        count[seq.child(i)]++;
    }

    EXPECT_EQ(4U, count.size());
    EXPECT_EQ(64, count[PuyoColor::RED]);
    EXPECT_EQ(64, count[PuyoColor::BLUE]);
    EXPECT_EQ(64, count[PuyoColor::YELLOW]);
    EXPECT_EQ(64, count[PuyoColor::GREEN]);
}

TEST(KumipuyoSeqGeneratorTest, withSeed)
{
    KumipuyoSeq seq0a = KumipuyoSeqGenerator::generateACPuyo2SequenceWithSeed(0);
    KumipuyoSeq seq0b = KumipuyoSeqGenerator::generateACPuyo2SequenceWithSeed(0);
    KumipuyoSeq seq1 = KumipuyoSeqGenerator::generateACPuyo2SequenceWithSeed(1);

    EXPECT_EQ(seq0a, seq0b);
    EXPECT_NE(seq0a, seq1);
}
