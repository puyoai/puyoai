#include "core/bit_field.h"

#include <gtest/gtest.h>
#include "core/core_field.h"

using namespace std;

TEST(BitFieldTest, constructor1)
{
    BitField bf;

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 14; ++y) {
            EXPECT_TRUE(bf.isEmpty(x, y));
        }
    }

    EXPECT_TRUE(bf.isColor(0, 0, PuyoColor::WALL));
    EXPECT_TRUE(bf.isColor(7, 1, PuyoColor::WALL));
}

TEST(BitFieldTest, constructor2)
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
                EXPECT_FALSE(bf.isEmpty(x, y));
            } else if (c == PuyoColor::EMPTY) {
                EXPECT_TRUE(bf.isEmpty(x, y));
            }
        }
    }

    EXPECT_TRUE(bf.isColor(0, 0, PuyoColor::WALL));
    EXPECT_TRUE(bf.isColor(7, 1, PuyoColor::WALL));
}

TEST(BitFieldTest, constructor3)
{
    BitField bf(
        "OOOOOO"
        "GGGYYY"
        "RRRBBB");

    EXPECT_TRUE(bf.isColor(1, 1, PuyoColor::RED));
    EXPECT_TRUE(bf.isColor(1, 2, PuyoColor::GREEN));
    EXPECT_TRUE(bf.isColor(1, 3, PuyoColor::OJAMA));
    EXPECT_TRUE(bf.isColor(4, 1, PuyoColor::BLUE));
    EXPECT_TRUE(bf.isColor(4, 2, PuyoColor::YELLOW));
    EXPECT_TRUE(bf.isColor(4, 3, PuyoColor::OJAMA));

    EXPECT_TRUE(bf.isColor(0, 0, PuyoColor::WALL));
    EXPECT_TRUE(bf.isColor(7, 1, PuyoColor::WALL));
}

TEST(BitFieldTest, setColor)
{
    static const PuyoColor colors[] = {
        PuyoColor::RED, PuyoColor::BLUE, PuyoColor::YELLOW, PuyoColor::GREEN,
        PuyoColor::OJAMA, PuyoColor::IRON
    };

    BitField bf;

    for (PuyoColor c : colors) {
        bf.setColor(1, 1, c);
        EXPECT_EQ(c, bf.color(1, 1)) << c;
        EXPECT_TRUE(bf.isColor(1, 1, c)) << c;
    }
}

TEST(BitFieldTest, isZenkeshi)
{
    BitField bf1;
    EXPECT_TRUE(bf1.isZenkeshi());

    BitField bf2("..O...");
    EXPECT_FALSE(bf2.isZenkeshi());

    BitField bf3("..R...");
    EXPECT_FALSE(bf3.isZenkeshi());
}

TEST(BitFieldTest, isConnectedPuyo)
{
    BitField bf(
        "B.B..Y"
        "RRRBBB");

    EXPECT_TRUE(bf.isConnectedPuyo(1, 1));
    EXPECT_TRUE(bf.isConnectedPuyo(2, 1));
    EXPECT_TRUE(bf.isConnectedPuyo(3, 1));
    EXPECT_TRUE(bf.isConnectedPuyo(4, 1));
    EXPECT_TRUE(bf.isConnectedPuyo(5, 1));
    EXPECT_TRUE(bf.isConnectedPuyo(6, 1));
    EXPECT_FALSE(bf.isConnectedPuyo(1, 2));
    EXPECT_FALSE(bf.isConnectedPuyo(3, 2));
    EXPECT_FALSE(bf.isConnectedPuyo(6, 2));
}

TEST(BitFieldTest, isConnectedPuyoEdge)
{
    BitField bf(
        ".....R" // 13
        "OOOOOR" // 12
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

    EXPECT_FALSE(bf.isConnectedPuyo(6, 12));
}

TEST(BitFieldTest, countConnectedPuyos)
{
    BitField bf(
        "RRRRRR"
        "BYBRRY"
        "RRRBBB");

    EXPECT_EQ(3, bf.countConnectedPuyos(1, 1));
    EXPECT_EQ(3, bf.countConnectedPuyos(4, 1));
    EXPECT_EQ(1, bf.countConnectedPuyos(1, 2));
    EXPECT_EQ(1, bf.countConnectedPuyos(3, 2));
    EXPECT_EQ(1, bf.countConnectedPuyos(6, 2));
    EXPECT_EQ(8, bf.countConnectedPuyos(4, 2));
}

TEST(BitFieldTest, countConnectedPuyosEdge)
{
    BitField bf(
        ".....R" // 13
        "OOOOOR" // 12
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

    EXPECT_EQ(1, bf.countConnectedPuyos(6, 12));
}

TEST(BitFieldTest, countConnectedPuyosWithChecked)
{
    BitField bf(
        "RRRRRR"
        "BYBRRY"
        "RRRBBB");

    FieldBits checked;
    EXPECT_EQ(8, bf.countConnectedPuyos(1, 3, &checked));
    EXPECT_TRUE(checked.get(1, 3));
    EXPECT_TRUE(checked.get(4, 2));
    EXPECT_TRUE(checked.get(5, 2));
    EXPECT_TRUE(checked.get(6, 3));
    EXPECT_FALSE(checked.get(1, 1));
    EXPECT_FALSE(checked.get(3, 1));
    EXPECT_FALSE(checked.get(1, 2));
    EXPECT_FALSE(checked.get(6, 2));
}

TEST(BitFieldTest, countConnectedPuyosMax4)
{
    BitField bf(
        "RRRRRR"
        "BYBRRY"
        "RRRBBB");

    EXPECT_EQ(3, bf.countConnectedPuyosMax4(1, 1));
    EXPECT_EQ(3, bf.countConnectedPuyosMax4(4, 1));
    EXPECT_EQ(1, bf.countConnectedPuyosMax4(1, 2));
    EXPECT_EQ(1, bf.countConnectedPuyosMax4(3, 2));
    EXPECT_EQ(1, bf.countConnectedPuyosMax4(6, 2));
    EXPECT_LE(4, bf.countConnectedPuyosMax4(4, 2));
}

TEST(BitFieldTest, countConnectedPuyosMax4Edge)
{
    BitField bf(
        ".....R" // 13
        "OOOOOR" // 12
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

    EXPECT_EQ(1, bf.countConnectedPuyosMax4(6, 12));
}

TEST(BitFieldTest, hasEmptyNeighbor)
{
    BitField bf(
        "RR..RR"
        "BYBRRY"
        "RRRBBB");

    EXPECT_TRUE(bf.hasEmptyNeighbor(2, 3));
    EXPECT_TRUE(bf.hasEmptyNeighbor(3, 2));

    EXPECT_FALSE(bf.hasEmptyNeighbor(1, 1));
    EXPECT_FALSE(bf.hasEmptyNeighbor(2, 1));
    EXPECT_FALSE(bf.hasEmptyNeighbor(6, 1));
}

TEST(BitFieldTest, hasEmptyNeighborEdge)
{
    BitField bf(
        "OOOOOR" // 12
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

    EXPECT_FALSE(bf.hasEmptyNeighbor(6, 12));
}

TEST(BitFieldTest, fillSameColorPosition)
{
    BitField bf(
        "RRRRRR"
        "BYBRRY"
        "RRRBBB");
    Position ps[128];
    FieldBits checked;
    Position* head = bf.fillSameColorPosition(1, 3, PuyoColor::RED, ps, &checked);

    std::sort(ps, head);
    EXPECT_EQ(8, head - ps);

    EXPECT_EQ(Position(1, 3), ps[0]);
    EXPECT_EQ(Position(2, 3), ps[1]);
    EXPECT_EQ(Position(3, 3), ps[2]);
    EXPECT_EQ(Position(4, 2), ps[3]);
    EXPECT_EQ(Position(4, 3), ps[4]);
    EXPECT_EQ(Position(5, 2), ps[5]);
    EXPECT_EQ(Position(5, 3), ps[6]);
    EXPECT_EQ(Position(6, 3), ps[7]);
}

TEST(BitFieldTest, fillSameColorPositionEdge)
{
    BitField bf(
        "RRRRRR" // 13
        "ROOOOR" // 12
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

    Position ps[128];
    FieldBits checked;
    Position* head = bf.fillSameColorPosition(6, 12, PuyoColor::RED, ps, &checked);

    EXPECT_EQ(1, head - ps);
    EXPECT_EQ(Position(6, 12), ps[0]);
}

TEST(BitFieldTest, simulate1)
{
    BitField bf(
        ".BBBB.");

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(40, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);

    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        EXPECT_TRUE(bf.isColor(x, 0, PuyoColor::WALL));
        EXPECT_TRUE(bf.isColor(x, 15, PuyoColor::WALL));
    }
}

TEST(BitFieldTest, simulate2)
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

TEST(BitFieldTest, simulate3)
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

TEST(BitFieldTest, simulate4)
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

TEST(BitFieldTest, simulate5)
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

TEST(BitFieldTest, simulate6)
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

TEST(BitFieldTest, simulate7)
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

TEST(BitFieldTest, simulate8)
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

TEST(BitFieldTest, vanish1)
{
    BitField bf(
        "RRBBBB"
        "RGRRBB");

    int score = bf.vanish(2);

    EXPECT_EQ(60 * 11, score);
    EXPECT_TRUE(bf.isEmpty(3, 2));
}

TEST(BitFieldTest, vanish2)
{
    BitField bf(
        "RRBB.B"
        "RGRRBB");

    int score = bf.vanish(2);
    EXPECT_EQ(0, score);
}

TEST(BitFieldTest, vanish3)
{
    BitField bf(
        "ROOOOR"
        "OBBBBO"
        "ROOOOR");

    int score = bf.vanish(1);

    EXPECT_EQ(40, score);
    EXPECT_TRUE(bf.isEmpty(1, 2));
    EXPECT_TRUE(bf.isEmpty(2, 2));
    EXPECT_TRUE(bf.isEmpty(3, 2));
    EXPECT_TRUE(bf.isEmpty(4, 2));
    EXPECT_TRUE(bf.isEmpty(5, 2));
    EXPECT_TRUE(bf.isEmpty(6, 2));
}

TEST(BitFieldTest, vanish4)
{
    BitField bf(
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

    int score = bf.vanish(1);
    EXPECT_EQ(60 * 3, score);

    EXPECT_TRUE(bf.isEmpty(1, 12));
    EXPECT_TRUE(bf.isEmpty(2, 12));
    EXPECT_TRUE(bf.isEmpty(3, 12));
    EXPECT_TRUE(bf.isEmpty(4, 12));
    EXPECT_TRUE(bf.isEmpty(5, 12));
    EXPECT_TRUE(bf.isEmpty(6, 12));

    EXPECT_FALSE(bf.isEmpty(1, 13));
    EXPECT_FALSE(bf.isEmpty(2, 13));
    EXPECT_TRUE(bf.isEmpty(3, 13));
    EXPECT_FALSE(bf.isEmpty(4, 13));
    EXPECT_FALSE(bf.isEmpty(5, 13));
    EXPECT_FALSE(bf.isEmpty(6, 13));
}

TEST(BitFieldTest, drop1)
{
    BitField bf(
        "RG...."
        "...G.."
        "GR...."
        "...GR.");
    BitField expected(
        "RG.G.."
        "GR.GR.");

    bf.drop();
    EXPECT_EQ(expected, bf);
}
