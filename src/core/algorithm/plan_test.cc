#include "core/algorithm/plan.h"

#include <gtest/gtest.h>
#include "core/field/core_field.h"
#include "core/kumipuyo.h"

using namespace std;

TEST(Plan, IteratorAvailablePlans)
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

