#include "core/client/ai/ai.h"

#include <gtest/gtest.h>

#include "core/frame_request.h"
#include "core/kumipuyo.h"

class TestAI : public AI {
public:
    TestAI() : AI("test") {}

    using AI::gameWillBegin;
    using AI::decisionRequested;
    using AI::grounded;
    using AI::enemyGrounded;
    using AI::enemyDecisionRequested;

    using AI::additionalThoughtInfo;

protected:
    virtual DropDecision think(int, const PlainField&, const KumipuyoSeq&,
                               const AdditionalThoughtInfo&)
    {
        return DropDecision(Decision(3, 0), "test");
    }

private:
    static const char* argv[];
};

class AITest : public testing::Test {
protected:
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
    req.playerFrameRequest[1].field = CoreField("  RR  ");
    ai.decisionRequested(req);
    ai.enemyDecisionRequested(req);
    EXPECT_TRUE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_FALSE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 4;
    req.playerFrameRequest[0].field = CoreField("  RRBB");
    req.playerFrameRequest[1].field = CoreField();
    ai.decisionRequested(req);
    ai.enemyDecisionRequested(req);
    EXPECT_TRUE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_TRUE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 5;
    req.playerFrameRequest[0].field = CoreField("  RRBB");
    req.playerFrameRequest[1].field = CoreField("  RRBB");
    ai.decisionRequested(req);
    ai.enemyDecisionRequested(req);
    EXPECT_TRUE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_TRUE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 6;
    req.playerFrameRequest[0].field = CoreField("  RRBB");
    req.playerFrameRequest[1].field = CoreField("  RRRR");
    ai.grounded(req);
    ai.enemyGrounded(req);
    EXPECT_TRUE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_FALSE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 7;
    req.playerFrameRequest[0].field = CoreField("RRRRBB");
    req.playerFrameRequest[1].field = CoreField("  RRRR");
    ai.grounded(req);
    ai.enemyGrounded(req);
    EXPECT_FALSE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_FALSE(ai.additionalThoughtInfo().enemyHasZenkeshi());

    req.frameId = 8;
    req.playerFrameRequest[0].field = CoreField();
    req.playerFrameRequest[1].field = CoreField();
    ai.decisionRequested(req);
    ai.enemyDecisionRequested(req);
    EXPECT_TRUE(ai.additionalThoughtInfo().hasZenkeshi());
    EXPECT_TRUE(ai.additionalThoughtInfo().enemyHasZenkeshi());
}
