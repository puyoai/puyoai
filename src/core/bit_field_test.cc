#include "core/bit_field.h"

#include <algorithm>
#include <gtest/gtest.h>

#include "core/plain_field.h"

using namespace std;

TEST(BitFieldTest, constructor1)
{
    BitField bf;

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 14; ++y) {
            EXPECT_TRUE(bf.isEmpty(x, y));
        }
    }

    for (int y = 0; y < 16; ++y) {
        EXPECT_TRUE(bf.isColor(0, y, PuyoColor::WALL));
        EXPECT_TRUE(bf.isColor(7, y, PuyoColor::WALL));
        EXPECT_EQ(PuyoColor::WALL, bf.color(0, y));
        EXPECT_EQ(PuyoColor::WALL, bf.color(7, y));
    }
    for (int x = 0; x < 8; ++x) {
        EXPECT_TRUE(bf.isColor(x, 0, PuyoColor::WALL));
        EXPECT_TRUE(bf.isColor(x, 15, PuyoColor::WALL));
        EXPECT_EQ(PuyoColor::WALL, bf.color(x, 0));
        EXPECT_EQ(PuyoColor::WALL, bf.color(x, 15));
    }
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

    for (int y = 0; y < 16; ++y) {
        EXPECT_TRUE(bf.isColor(0, y, PuyoColor::WALL));
        EXPECT_TRUE(bf.isColor(7, y, PuyoColor::WALL));
        EXPECT_EQ(PuyoColor::WALL, bf.color(0, y));
        EXPECT_EQ(PuyoColor::WALL, bf.color(7, y));
    }
    for (int x = 0; x < 8; ++x) {
        EXPECT_TRUE(bf.isColor(x, 0, PuyoColor::WALL));
        EXPECT_TRUE(bf.isColor(x, 15, PuyoColor::WALL));
        EXPECT_EQ(PuyoColor::WALL, bf.color(x, 0));
        EXPECT_EQ(PuyoColor::WALL, bf.color(x, 15));
    }
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

    for (int y = 0; y < 16; ++y) {
        EXPECT_TRUE(bf.isColor(0, y, PuyoColor::WALL));
        EXPECT_TRUE(bf.isColor(7, y, PuyoColor::WALL));
        EXPECT_EQ(PuyoColor::WALL, bf.color(0, y));
        EXPECT_EQ(PuyoColor::WALL, bf.color(7, y));
    }
    for (int x = 0; x < 8; ++x) {
        EXPECT_TRUE(bf.isColor(x, 0, PuyoColor::WALL));
        EXPECT_TRUE(bf.isColor(x, 15, PuyoColor::WALL));
        EXPECT_EQ(PuyoColor::WALL, bf.color(x, 0));
        EXPECT_EQ(PuyoColor::WALL, bf.color(x, 15));
    }
}

TEST(BitFieldTest, isNormalColor)
{
    BitField bf("RBYGO.");

    EXPECT_TRUE(bf.isNormalColor(1, 1));
    EXPECT_TRUE(bf.isNormalColor(2, 1));
    EXPECT_TRUE(bf.isNormalColor(3, 1));
    EXPECT_TRUE(bf.isNormalColor(4, 1));
    EXPECT_FALSE(bf.isNormalColor(5, 1));
    EXPECT_FALSE(bf.isNormalColor(6, 1));
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

TEST(BitFieldTest, countConnection)
{
    BitField bf(
        "R...GG" // 13
        "RROOOG" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "YYGGYY"
        "RRRBBB"
    );

    int count2, count3;
    bf.countConnection(&count2, &count3);

    EXPECT_EQ(4, count2);
    EXPECT_EQ(2, count3);
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
    BitField bf(
        "BBBBBB");

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(60 * 3, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST(BitFieldTest, simulate3)
{
    BitField bf(
        "YYYY.."
        "BBBB..");

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(80 * 3, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST(BitFieldTest, simulate4)
{
    BitField bf(
        "YYYYYY"
        "BBBBBB");

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(120 * (3 + 3 + 3), result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST(BitFieldTest, simulate5)
{
    BitField bf(
        ".YYYG."
        "BBBBY.");

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
    BitField bf(
        ".RBRB."
        "RBRBR."
        "RBRBR."
        "RBRBRR");

    RensaResult result = bf.simulate();
    EXPECT_EQ(5, result.chains);
    EXPECT_EQ(40 + 40 * 8 + 40 * 16 + 40 * 32 + 40 * 64, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION * 5 +
              FRAMES_TO_DROP_FAST[3] * 4 +
              FRAMES_GROUNDING * 4, result.frames);
    EXPECT_TRUE(result.quick);
}

TEST(BitFieldTest, simulate7)
{
    BitField bf(
        ".YGGY."
        "BBBBBB"
        "GYBBYG"
        "BBBBBB");

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(140 * 10, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION +
              FRAMES_TO_DROP_FAST[3] +
              FRAMES_GROUNDING, result.frames);
    EXPECT_FALSE(result.quick);
}

TEST(BitFieldTest, simulate8)
{
    BitField bf(
        "BBBBBB"
        "GYBBYG"
        "BBBBYB");

    RensaResult result = bf.simulate();
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(120 * 10, result.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION +
              FRAMES_TO_DROP_FAST[1] +
              FRAMES_GROUNDING, result.frames);
    EXPECT_FALSE(result.quick);
}

TEST(BitFieldTest, simulateWithTracker1)
{
    BitField bf(
        "..RR.."
        "BBBBRR");

    RensaYPositionTracker tracker;
    bf.simulate(&tracker);

    EXPECT_EQ(2, tracker.originalY(1, 1));
    EXPECT_EQ(3, tracker.originalY(3, 1));
}

TEST(BitFieldTest, vanishDrop1)
{
    BitField bf(
        "..RR.."
        "BBBBRR");

    RensaYPositionTracker tracker;
    BitField::SimulationContext context;
    RensaStepResult stepResult = bf.vanishDrop(&context, &tracker);

    EXPECT_EQ(40, stepResult.score);

    BitField expected(
        "..RRRR");

    EXPECT_EQ(expected, bf);
    EXPECT_EQ(2, context.currentChain);
    EXPECT_EQ(2, tracker.originalY(1, 1));
    EXPECT_EQ(2, tracker.originalY(3, 1));
    EXPECT_EQ(1, tracker.originalY(6, 1));
}

TEST(BitFieldTest, vanishDrop2)
{
    BitField bf("....YY");

    RensaYPositionTracker tracker;
    BitField::SimulationContext context;
    RensaStepResult stepResult = bf.vanishDrop(&context, &tracker);

    EXPECT_EQ(0, stepResult.score);

    BitField expected("....YY");

    EXPECT_EQ(expected, bf);
    EXPECT_EQ(1, context.currentChain);
    EXPECT_EQ(1, tracker.originalY(1, 1));
    EXPECT_EQ(1, tracker.originalY(3, 1));
    EXPECT_EQ(1, tracker.originalY(6, 1));
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

TEST(BitFieldTest, ignitionPuyoBits)
{
    BitField bf(
        "..YY.."
        "GGGGYY"
        "RRRROY");

    FieldBits bits(
        "1111.."
        "11111.");

    EXPECT_EQ(bits, bf.ignitionPuyoBits());
}

TEST(BitFieldTest, calculateHeight)
{
    alignas(16) std::uint16_t heights[FieldConstant::MAP_WIDTH];

    BitField bf(
        ".....O" // 14
        "....OO"
        "...OOO" // 12
        "..OOOO"
        ".OOOOO"
        ".OOOOO"
        ".OOOOO" // 8
        ".OOOOO"
        ".OOOOO"
        ".OOOOO"
        ".OOOOO" // 4
        ".OOOOO"
        ".OOOOO"
        ".OOOOO"
    );
    bf.calculateHeight(heights);

    EXPECT_EQ(0, heights[1]);
    EXPECT_EQ(10, heights[2]);
    EXPECT_EQ(11, heights[3]);
    EXPECT_EQ(12, heights[4]);
    EXPECT_EQ(13, heights[5]);
    EXPECT_EQ(13, heights[6]); // not 14, but 13.
}
