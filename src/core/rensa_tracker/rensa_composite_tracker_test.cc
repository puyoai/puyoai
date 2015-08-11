#include "core/rensa_tracker/rensa_composite_tracker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/rensa_tracker/rensa_chain_tracker.h"
#include "core/rensa_tracker/rensa_coef_tracker.h"

TEST(RensaCompositeTracker, track)
{
    const CoreField original(
        ".RBGY."
        "RBGYR."
        "RBGYR."
        "RBGYRR");

    RensaCoefTracker coefTracker1, coefTracker2;
    RensaChainTracker chainTracker1, chainTracker2;
    RensaCompositeTracker<RensaCoefTracker, RensaChainTracker> tracker(&coefTracker2, &chainTracker2);

    CoreField cf1(original);
    cf1.simulate(&coefTracker1);
    CoreField cf2(original);
    cf2.simulate(&chainTracker1);

    CoreField cf3(original);
    cf3.simulate(&tracker);

    EXPECT_EQ(5, chainTracker1.result().erasedAt(1, 1));
    EXPECT_EQ(5, chainTracker2.result().erasedAt(1, 1));

    EXPECT_EQ(4, coefTracker1.result().numErased(5));
    EXPECT_EQ(4, coefTracker2.result().numErased(5));
}
