#include "core/plan/plan.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
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
    EXPECT_EQ(0, plan.framesToIgnite());
}

TEST(Plan, iterateAvailablePlansWithEvents)
{
    CoreField field;
    KumipuyoSeq seq("BBBB");
    bool found = false;

    auto callback = [&found](const RefPlan& p) {
        if (p.isRensaPlan())
            found = true;
    };

    // With |events|, 2 rows of Ojama will fall just after 1st control,
    // so we cannot vanish puyos, because of ojama wall.
    std::vector<Plan::Event> events = {Plan::Event::fallOjamaRowsEvent(1, 2)};
    found = false;
    Plan::iterateAvailablePlansWithEvents(field, seq, 2, events, callback);
    EXPECT_FALSE(found);

    // With |lateEvents|, Ojama will fall 1000 frames later.  It means we can control
    // 2 Kumipuyos before Ojama fall, and we can vanish puyos.
    std::vector<Plan::Event> lateEvents = {Plan::Event::fallOjamaRowsEvent(1000, 2)};
    found = false;
    Plan::iterateAvailablePlansWithEvents(field, seq, 2, lateEvents, callback);
    EXPECT_TRUE(found);
}

TEST(Plan, iterateAvailablePlansWithMultipleEvents)
{
    CoreField field;
    KumipuyoSeq seq("BBBB");
    bool found = false;
    
    auto callback = [&found](const RefPlan& p) {
        if (p.isRensaPlan())
            found = true;
    };

    // Two rows of ojama in total will fall before 2nd control.
    std::vector<Plan::Event> events = {
        Plan::Event::fallOjamaRowsEvent(1, 1),
        Plan::Event::fallOjamaRowsEvent(10, 1)
    };
    found = false;
    Plan::iterateAvailablePlansWithEvents(field, seq, 2, events, callback);
    EXPECT_FALSE(found);

    // A row of ojama fall before 2nd control, and another row of
    // ojama will fall after 2nd control.
    std::vector<Plan::Event> lateEvents = {
        Plan::Event::fallOjamaRowsEvent(1, 1),
        Plan::Event::fallOjamaRowsEvent(1000, 1)
    };
    found = false;
    Plan::iterateAvailablePlansWithEvents(field, seq, 2, lateEvents, callback);
    EXPECT_TRUE(found);
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
