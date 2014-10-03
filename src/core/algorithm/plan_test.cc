#include "core/algorithm/plan.h"

#include <gtest/gtest.h>
#include "core/constant.h"
#include "core/field/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

TEST(Plan, iterateAvailablePlans)
{
    CoreField field("  YY  ");
    KumipuyoSeq kumipuyoSeq = {
        Kumipuyo(PuyoColor::RED, PuyoColor::RED),
        Kumipuyo(PuyoColor::RED, PuyoColor::RED),
    };

    // This 2-rensa should exist.
    //   RR
    // RRYYYY

    bool found = false;
    Plan::iterateAvailablePlans(field, kumipuyoSeq, 3, [&found](const RefPlan& plan){
        if (!plan.isRensaPlan())
            return;
        if (plan.rensaResult().chains < 2)
            return;
        if (plan.field().isZenkeshi())
            found = true;
    });

    EXPECT_TRUE(found);
}

TEST(Plan, iterateAvailablePlansWithRensa)
{
    CoreField field("  RR  ");
    KumipuyoSeq seq("RRBB");

    bool found = false;
    Plan plan;
    Plan::iterateAvailablePlans(field, seq, 2, [&found, &plan](const RefPlan& p) {
        if (p.decisions().size() == 1 && p.decisions().front() == Decision(3, 2)) {
            found = true;
            plan = p.toPlan();
        }
    });

    EXPECT_TRUE(found);
    EXPECT_EQ(1, plan.rensaResult().chains);
    EXPECT_EQ(40, plan.rensaResult().score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, plan.rensaResult().frames);
    EXPECT_TRUE(plan.rensaResult().quick);

    EXPECT_EQ(FRAMES_TO_DROP_FAST[10] + FRAMES_GROUNDING + FRAMES_VANISH_ANIMATION,
              plan.totalFrames());
    EXPECT_EQ(0, plan.framesToInitiate());
}

TEST(Plan, numChigiri)
{
    CoreField cf("  O   ");
    KumipuyoSeq seq("RRRR");

    bool found;
    Plan::iterateAvailablePlans(cf, seq, 2, [&](const RefPlan& plan) {
            if (plan.decision(0) == Decision(3, 1) && plan.decision(1) == Decision(3, 1)) {
                EXPECT_EQ(2, plan.numChigiri());
                found = true;
            }
    });

    EXPECT_TRUE(found);
}
