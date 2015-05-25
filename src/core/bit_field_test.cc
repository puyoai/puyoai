#include "core/bit_field.h"

#include <gtest/gtest.h>
#include "core/core_field.h"

using namespace std;

class BitFieldTest : public testing::Test {
protected:
    int vanish(BitField* bf, int chain, FieldBits* erased) {
        return bf->vanish(chain, erased);
    }

    void drop(BitField* bf, const FieldBits& erased) {
        bf->drop(erased);
    }
};

TEST_F(BitFieldTest, ctor)
{
    PlainField pf(
        "OOOOOO"
        "GGGYYY"
        "RRRBBB");

    BitField bf(pf);

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            PuyoColor c = pf.color(x, y);
            if (isNormalColor(c) || c == PuyoColor::OJAMA) {
                EXPECT_TRUE(bf.isColor(x, y, c)) << x << ' ' << y << ' ' << c;
            } else if (c == PuyoColor::EMPTY) {
                EXPECT_TRUE(bf.isEmpty(x, y));
            }
        }
    }
}

TEST_F(BitFieldTest, simulate1)
{
    CoreField cf(
        ".BBBB.");
    BitField bf(cf);

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(40, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST_F(BitFieldTest, simulate2)
{
    CoreField cf(
        "BBBBBB");
    BitField bf(cf);

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(60 * 3, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST_F(BitFieldTest, simulate3)
{
    CoreField cf(
        "YYYY.."
        "BBBB..");
    BitField bf(cf);

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(80 * 3, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST_F(BitFieldTest, simulate4)
{
    CoreField cf(
        "YYYYYY"
        "BBBBBB");
    BitField bf(cf);

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(120 * (3 + 3 + 3), result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST_F(BitFieldTest, simulate5)
{
    CoreField cf(
        ".YYYG."
        "BBBBY.");
    BitField bf(cf);

    RensaResult result = bf.simulate();
    EXPECT_EQ(2, result.chains);
    EXPECT_EQ(40 + 40 * 8, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION * 2 +
              FRAMES_TO_DROP_FAST[1] * 2 +
              FRAMES_GROUNDING * 2, result.frames);
    EXPECT_FALSE(result.quick);
}

TEST_F(BitFieldTest, simulate6)
{
    CoreField cf(".RBRB."
                 "RBRBR."
                 "RBRBR."
                 "RBRBRR");
    BitField bf(cf);

    RensaResult cfResult = cf.simulate();
    RensaResult bfResult = bf.simulate();
    EXPECT_EQ(cfResult, bfResult);
}

TEST_F(BitFieldTest, simulate7)
{
    CoreField cf(
        ".YGGY."
        "BBBBBB"
        "GYBBYG"
        "BBBBBB");
    BitField bf(cf);

    RensaResult cfResult = cf.simulate();
    RensaResult bfResult = bf.simulate();
    EXPECT_EQ(cfResult, bfResult);
}

TEST_F(BitFieldTest, simulate8)
{
    CoreField cf(
        "BBBBBB"
        "GYBBYG"
        "BBBBYB");
    BitField bf(cf);

    RensaResult cfResult = cf.simulate();
    RensaResult bfResult = bf.simulate();
    EXPECT_EQ(cfResult, bfResult);
}

TEST_F(BitFieldTest, vanish1)
{
    PlainField pf(
        "RRBBBB"
        "RGRRBB");
    BitField bf(pf);

    FieldBits erased;
    int score = vanish(&bf, 2, &erased);

    EXPECT_EQ(60 * 11, score);
    EXPECT_EQ(FieldBits(pf, PuyoColor::BLUE), erased);
}

TEST_F(BitFieldTest, vanish2)
{
    PlainField pf(
        "RRBB.B"
        "RGRRBB");
    BitField bf(pf);

    FieldBits erased;
    int score = vanish(&bf, 2, &erased);

    EXPECT_EQ(0, score);
}

TEST_F(BitFieldTest, vanish3)
{
    PlainField pf(
        "ROOOOR"
        "OBBBBO"
        "ROOOOR");
    BitField bf(pf);

    FieldBits erased;
    int score = vanish(&bf, 1, &erased);

    EXPECT_EQ(40, score);
    EXPECT_TRUE(erased.get(1, 2));
    EXPECT_TRUE(erased.get(2, 2));
    EXPECT_TRUE(erased.get(3, 2));
    EXPECT_TRUE(erased.get(4, 2));
    EXPECT_TRUE(erased.get(5, 2));
    EXPECT_TRUE(erased.get(6, 2));
}

TEST_F(BitFieldTest, vanish4)
{
    PlainField pf(
        "RR.RRR" // 13
        "RRRRRR" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");
    BitField bf(pf);

    FieldBits erased;
    int score = vanish(&bf, 1, &erased);
    EXPECT_EQ(60 * 3, score);

    EXPECT_TRUE(erased.get(1, 12));
    EXPECT_TRUE(erased.get(2, 12));
    EXPECT_TRUE(erased.get(3, 12));
    EXPECT_TRUE(erased.get(4, 12));
    EXPECT_TRUE(erased.get(5, 12));
    EXPECT_TRUE(erased.get(6, 12));

    EXPECT_FALSE(erased.get(1, 13));
    EXPECT_FALSE(erased.get(2, 13));
    EXPECT_FALSE(erased.get(3, 13));
    EXPECT_FALSE(erased.get(4, 13));
    EXPECT_FALSE(erased.get(5, 13));
    EXPECT_FALSE(erased.get(6, 13));
}

TEST_F(BitFieldTest, vanishdrop1)
{
    PlainField pf(
        "RGBBBB"
        "BBBGRB"
        "GRBBBB"
        "BBBGRB");
    BitField bf(pf);

    FieldBits erased;
    vanish(&bf, 2, &erased);
    drop(&bf, erased);

    FieldBits red = bf.bits(PuyoColor::RED);
    EXPECT_TRUE(red.get(1, 2));
    EXPECT_TRUE(red.get(2, 1));
    EXPECT_TRUE(red.get(5, 1));
    EXPECT_TRUE(red.get(5, 2));
    EXPECT_FALSE(red.get(1, 1));
    EXPECT_FALSE(red.get(1, 4));

    FieldBits green = bf.bits(PuyoColor::GREEN);
    EXPECT_TRUE(green.get(1, 1));
    EXPECT_TRUE(green.get(2, 2));
    EXPECT_TRUE(green.get(4, 1));
    EXPECT_TRUE(green.get(4, 2));
    EXPECT_FALSE(green.get(5, 1));
    EXPECT_FALSE(green.get(6, 1));
}
