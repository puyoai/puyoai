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
    using AI::decisionRequested;
    using AI::grounded;
    using AI::enemyGrounded;
    using AI::enemyDecisionRequested;

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

    static void mergeField(CoreField* ours, const PlainField& provided)
    {
        AI::mergeField(ours, provided);
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
    ai_.decisionRequested(req);
    ai_.enemyDecisionRequested(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);

    req.frameId = 3;
    req.playerFrameRequest[0].field = CoreField("  RR  ");
    req.playerFrameRequest[1].field = CoreField("  RR  ");
    ai_.grounded(req);
    ai_.enemyGrounded(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);

    req.frameId = 4;
    ai_.decisionRequested(req);
    ai_.enemyDecisionRequested(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);

    req.frameId = 5;
    req.playerFrameRequest[0].field = CoreField("  RRRR");
    req.playerFrameRequest[1].field = CoreField("  RRRR");
    ai_.grounded(req);
    ai_.enemyGrounded(req);
    EXPECT_TRUE(myPlayerState().hasZenkeshi);
    EXPECT_TRUE(enemyPlayerState().hasZenkeshi);

    req.frameId = 6;
    ai_.decisionRequested(req);
    ai_.enemyDecisionRequested(req);
    EXPECT_TRUE(myPlayerState().hasZenkeshi);
    EXPECT_TRUE(enemyPlayerState().hasZenkeshi);

    req.frameId = 7;
    req.playerFrameRequest[0].field = CoreField(" BRRRR");
    req.playerFrameRequest[1].field = CoreField(" BRRRR");
    ai_.grounded(req);
    ai_.enemyGrounded(req);
    EXPECT_FALSE(myPlayerState().hasZenkeshi);
    EXPECT_FALSE(enemyPlayerState().hasZenkeshi);
}

TEST_F(AITest, ojamaCount)
{
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
    ai_.decisionRequested(req);
    ai_.enemyDecisionRequested(req);
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
    ai_.grounded(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(32, enemyPlayerState().pendingOjama);

    req.frameId = 4;
    req.playerFrameRequest[1].field = CoreField(
        "RBRBR "
        "BRBRB "
        "BRBRB "
        "BRBRBB");
    ai_.enemyGrounded(req);
    EXPECT_EQ(0, myPlayerState().fixedOjama);
    EXPECT_EQ(37, myPlayerState().pendingOjama);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
    EXPECT_EQ(0, enemyPlayerState().pendingOjama);

    req.frameId = 5;
    req.playerFrameRequest[0].field = CoreField("R     ");
    req.playerFrameRequest[1].field = CoreField(" B    ");
    // don't call ai_.decisionRequested(req); here.
    ai_.enemyDecisionRequested(req);
    EXPECT_EQ(37, myPlayerState().fixedOjama);
    EXPECT_EQ(0, myPlayerState().pendingOjama);
    EXPECT_EQ(0, enemyPlayerState().fixedOjama);
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
        "   OOO"
        "RRRGGG");

    CoreField f3(
        "OOO"
        "RRRGGG");

    {
        CoreField f = f1;
        mergeField(&f, f2);
        EXPECT_EQ(f2, f);
    }
    {
        CoreField f = f1;
        mergeField(&f, f3);
        EXPECT_EQ(f3, f);
    }
    {
        CoreField f = f2;
        mergeField(&f, f3);
        EXPECT_EQ(f3, f);
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
        CoreField f = f1;
        mergeField(&f, f2);
        EXPECT_EQ(f1, f);
    }
    {
        CoreField f = f1;
        mergeField(&f, f3);
        EXPECT_EQ(f3, f);
    }
    {
        CoreField f = f1;
        mergeField(&f, f4);
        EXPECT_EQ(f3, f);
    }
}
