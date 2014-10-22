#include "core/client/ai/ai.h"

#include <gtest/gtest.h>

#include "core/frame_request.h"
#include "core/kumipuyo.h"

class TestAI : public AI {
public:
    TestAI() : AI("test") {}
    virtual ~TestAI() {}

    using AI::gameWillBegin;
    using AI::decisionRequested;
    using AI::grounded;
    using AI::enemyGrounded;
    using AI::enemyDecisionRequested;

    using AI::additionalThoughtInfo;

protected:
    virtual DropDecision think(int, const CoreField&, const KumipuyoSeq&,
                               const AdditionalThoughtInfo&, bool)
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

    TestAI ai;
};

TEST_F(AITest, zenkeshi)
{
    KumipuyoSeq seq("RRRRBBGG");

    CoreField field;
    FrameRequest req;

    req.frameId = 1;
    ai.gameWillBegin(req);
    EXPECT_FALSE(ai.additionalThoughtInfo().hasZenkeshi());

    req.frameId = 2;
    ai.decisionRequested(req);
    ai.enemyDecisionRequested(req);
    EXPECT_FALSE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_FALSE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 3;
    req.playerFrameRequest[0].field = CoreField("  RR  ");
    req.playerFrameRequest[1].field = CoreField("  RR  ");
    ai.grounded(req);
    ai.enemyGrounded(req);
    EXPECT_FALSE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_FALSE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 4;
    ai.decisionRequested(req);
    ai.enemyDecisionRequested(req);
    EXPECT_FALSE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_FALSE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 5;
    req.playerFrameRequest[0].field = CoreField("  RRRR");
    req.playerFrameRequest[1].field = CoreField("  RRRR");
    ai.grounded(req);
    ai.enemyGrounded(req);
    EXPECT_TRUE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_TRUE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 6;
    ai.decisionRequested(req);
    ai.enemyDecisionRequested(req);
    EXPECT_TRUE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_TRUE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 7;
    req.playerFrameRequest[0].field = CoreField(" BRRRR");
    req.playerFrameRequest[1].field = CoreField(" BRRRR");
    ai.grounded(req);
    ai.enemyGrounded(req);
    EXPECT_FALSE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_FALSE(ai.additionalThoughtInfo().enemyHasZenkeshi());
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
