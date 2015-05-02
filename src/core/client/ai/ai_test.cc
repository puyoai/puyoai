#include "core/client/ai/ai.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/frame_request.h"
#include "core/plain_field.h"

class TestAI : public AI {
public:
    TestAI() : AI("test") {}
    virtual ~TestAI() {}

    using AI::gameWillBegin;
    using AI::decisionRequestedForMe;
    using AI::groundedForMe;
    using AI::puyoErasedForMe;
    using AI::puyoErasedForEnemy;
    using AI::groundedForEnemy;
    using AI::decisionRequestedForEnemy;

protected:
    virtual DropDecision think(int, const CoreField&, const KumipuyoSeq&,
                               const PlayerState&, const PlayerState&, bool) const override
    {
        return DropDecision(Decision(3, 0), "test");
    }

private:
    static const char* argv[];
};

class AITest : public testing::Test {
protected:
    static bool isFieldInconsistent(const PlainField& lhs, const PlainField& rhs)
    {
        return AI::isFieldInconsistent(lhs, rhs);
    }

    static CoreField mergeField(const CoreField& ours, const PlainField& provided, bool ojamaDropped)
    {
        return AI::mergeField(ours, provided, ojamaDropped);
    }

    const PlayerState& myPlayerState() { return ai_.myPlayerState(); }
    const PlayerState& enemyPlayerState() { return ai_.enemyPlayerState(); }

    TestAI ai_;
};

TEST_F(AITest, zenkeshi)
{
    KumipuyoSeq seq("RRRRBBGG");

    CoreField field;
    FrameRequest req;

    req.frameId = 1;
    ai_.gameWillBegin(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);

    req.frameId = 2;
    ai_.decisionRequestedForMe(req);
    ai_.decisionRequestedForEnemy(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);

    req.frameId = 3;
    req.playerFrameRequest[0].field = CoreField("  RR  ");
    req.playerFrameRequest[1].field = CoreField("  RR  ");
    ai_.groundedForMe(req);
    ai_.groundedForEnemy(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);

    req.frameId = 4;
    ai_.decisionRequestedForMe(req);
    ai_.decisionRequestedForEnemy(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);

    req.frameId = 5;
    req.playerFrameRequest[0].field = CoreField("  RRRR");
    req.playerFrameRequest[1].field = CoreField("  RRRR");
    ai_.groundedForMe(req);
    ai_.groundedForEnemy(req);
    EXPECT_TRUE(myPlayerState().hasZenkeshi);
    EXPECT_TRUE(enemyPlayerState().hasZenkeshi);

    req.frameId = 6;
    req.playerFrameRequest[0].field = CoreField("  RRRR");
    req.playerFrameRequest[1].field = CoreField("  RRRR");
    ai_.puyoErasedForMe(req);
    ai_.puyoErasedForEnemy(req);
    EXPECT_TRUE(myPlayerState().hasZenkeshi);
    EXPECT_TRUE(enemyPlayerState().hasZenkeshi);

    req.frameId = 7;
    ai_.decisionRequestedForMe(req);
    ai_.decisionRequestedForEnemy(req);
    EXPECT_TRUE(myPlayerState().hasZenkeshi);
    EXPECT_TRUE(enemyPlayerState().hasZenkeshi);

    req.frameId = 8;
    req.playerFrameRequest[0].field = CoreField(" BRRRR");
    req.playerFrameRequest[1].field = CoreField(" BRRRR");
    ai_.groundedForMe(req);
    ai_.groundedForEnemy(req);
    EXPECT_TRUE(myPlayerState().hasZenkeshi);
    EXPECT_TRUE(enemyPlayerState().hasZenkeshi);

    req.frameId = 9;
    req.playerFrameRequest[0].field = CoreField(" BRRRR");
    req.playerFrameRequest[1].field = CoreField(" BRRRR");
    ai_.puyoErasedForMe(req);
    ai_.puyoErasedForEnemy(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);
}

TEST_F(AITest, ojamaCount)
{
    CoreField original(
        "RBRB  "
        "BRBR  "
        "BRBR  "
        "BRBRR ");
    RensaResult originalRensaResult = original.simulate();

    KumipuyoSeq seq("RRRRBBGG");

    CoreField field;
    FrameRequest req;

    req.frameId = 1;
    ai_.gameWillBegin(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(0, enemyPlayerState().pendingOjama);

    req.frameId = 2;
    ai_.decisionRequestedForMe(req);
    ai_.decisionRequestedForEnemy(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(0, enemyPlayerState().pendingOjama);

    req.frameId = 3;
    req.playerFrameRequest[0].field = CoreField(
        "RBRB  "
        "BRBR  "
        "BRBR  "
        "BRBRR ");
    ai_.groundedForMe(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(0, enemyPlayerState().pendingOjama);

    // score = 40
    req.frameId = 4;
    req.playerFrameRequest[0].field = CoreField(
        "RBRB  "
        "BRBR  "
        "BRBR  "
        "BRBRR ");
    ai_.puyoErasedForMe(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    EXPECT_EQ(40, myPlayerState().unusedScore);
    originalRensaResult.score -= 40;
    EXPECT_EQ(originalRensaResult, myPlayerState().currentRensaResult);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(0, enemyPlayerState().pendingOjama);

    // score = 40 + 40 * 8 = 360
    req.frameId = 5;
    req.playerFrameRequest[0].field = CoreField(
        "RBR   "
        "BRB   "
        "BRB   "
        "BRBB ");
    ai_.puyoErasedForMe(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    originalRensaResult.score -= 320;
    originalRensaResult.frames -= 3 + FRAMES_GROUNDING + FRAMES_VANISH_ANIMATION;
    EXPECT_EQ(originalRensaResult, myPlayerState().currentRensaResult);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(5, enemyPlayerState().pendingOjama);

    // score = 360 + 40 * 16 = 1000
    req.frameId = 6;
    req.playerFrameRequest[0].field = CoreField(
        "RB    "
        "BR    "
        "BR    "
        "BRR   ");
    ai_.puyoErasedForMe(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    originalRensaResult.score -= 40 * 16;
    originalRensaResult.frames -= 3 + FRAMES_GROUNDING + FRAMES_VANISH_ANIMATION;
    EXPECT_EQ(originalRensaResult, myPlayerState().currentRensaResult);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(14, enemyPlayerState().pendingOjama);

    // score = 1000 + 40 * 32 = 2280
    req.frameId = 7;
    req.playerFrameRequest[0].field = CoreField(
        "R     "
        "B     "
        "B     "
        "BB    ");
    ai_.puyoErasedForMe(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    originalRensaResult.score -= 40 * 32;
    originalRensaResult.frames -= 3 + FRAMES_GROUNDING + FRAMES_VANISH_ANIMATION;
    EXPECT_EQ(originalRensaResult, myPlayerState().currentRensaResult);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(32, enemyPlayerState().pendingOjama);

    req.frameId = 8;
    req.playerFrameRequest[0].field = CoreField(
        "      "
        "      "
        "      "
        "R     ");
    ai_.decisionRequestedForMe(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    EXPECT_EQ(32, enemyPlayerState().fixedOjama);
    EXPECT_EQ(0, enemyPlayerState().pendingOjama);
}

TEST_F(AITest, isFieldInconsistent)
{
    PlainField f1(
        "OO OOO" // 12
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

    PlainField f2(
        "OO OOO" // 13
        "OO OOO" // 12
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

    PlainField f3(
        "OO OO " // 13
        "OO OO " // 12
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

    EXPECT_FALSE(isFieldInconsistent(f1, f2));
    EXPECT_TRUE(isFieldInconsistent(f1, f3));
    EXPECT_TRUE(isFieldInconsistent(f2, f3));
}

TEST_F(AITest, mergeFieldSimpleCase)
{
    CoreField f1(
        "RRRGGG");

    CoreField f2(
        "...OOO"
        "RRRGGG");

    CoreField f3(
        "OOO..."
        "RRRGGG");

    {
        CoreField cf = mergeField(f1, f2, false);
        EXPECT_EQ(f2, cf);
    }
    {
        CoreField cf = mergeField(f1, f3, false);
        EXPECT_EQ(f3, cf);
    }
    {
        CoreField cf = mergeField(f2, f3, false);
        EXPECT_EQ(f3, cf);
    }
}

TEST_F(AITest, mergeField)
{
    CoreField f1(
        "OO OOO" // 13
        "OO OOO" // 12
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

    CoreField f2(
        "OO OOO" // 12
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

    CoreField f3(
        "OO OO " // 13
        "OO OO " // 12
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

    CoreField f4(
        "OO OO " // 12
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

    {
        CoreField cf = mergeField(f1, f2, false);
        EXPECT_EQ(f1, cf);
    }
    {
        CoreField cf = mergeField(f1, f3, false);
        EXPECT_EQ(f3, cf);
    }
    {
        CoreField cf = mergeField(f1, f4, false);
        EXPECT_EQ(f3, cf);
    }
}

TEST_F(AITest, mergeFieldOjama)
{
    CoreField original(
        "OO OOO" // 10
        "OOOOOO" // 9
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    CoreField provided1(
        "OO OOO" // 12
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

    CoreField expected1(
        "OO OOO" // 13
        "OO OOO" // 12
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

    CoreField provided2(
        "OO OOO" // 12
        "OO OOO"
        "OOOOOO" // 10
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    CoreField expected2(
        "OO OOO" // 12
        "OO OOO" // 11
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

    {
        CoreField cf = mergeField(original, provided1, true);
        EXPECT_EQ(expected1, cf);
    }
    {
        CoreField cf = mergeField(original, provided2, true);
        EXPECT_EQ(expected2, cf);
    }
}
