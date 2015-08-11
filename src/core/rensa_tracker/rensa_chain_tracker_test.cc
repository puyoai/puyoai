#include "core/rensa_tracker/rensa_chain_tracker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

TEST(RensaChainTrackResult, initial)
{
    RensaChainTrackResult rtr;
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        for (int y = 1; y <= FieldConstant::HEIGHT; ++y) {
            EXPECT_EQ(0, rtr.erasedAt(x, y)) << "x=" << x << " y=" << y;
        }
    }
}

TEST(RensaChainTrackResult, constructor)
{
    RensaChainTrackResult rtr(
        "aB...."
        "12....");

    EXPECT_EQ(1, rtr.erasedAt(1, 1));
    EXPECT_EQ(2, rtr.erasedAt(2, 1));
    EXPECT_EQ(10, rtr.erasedAt(1, 2));
    EXPECT_EQ(11, rtr.erasedAt(2, 2));
    EXPECT_EQ(0, rtr.erasedAt(3, 1));
}

TEST(RensaChainTrackerTest, TrackedCoreFieldSimulation)
{
    CoreField f("400040"
                "456474"
                "445667"
                "556774");


    RensaChainTracker tracker;
    RensaResult basicRensaResult = f.simulate(&tracker);

    const RensaChainTrackResult& trackResult = tracker.result();

    EXPECT_EQ(5, basicRensaResult.chains);
    EXPECT_EQ(1, trackResult.erasedAt(1, 2));
    EXPECT_EQ(2, trackResult.erasedAt(1, 1));
    EXPECT_EQ(3, trackResult.erasedAt(3, 3));
    EXPECT_EQ(4, trackResult.erasedAt(5, 3));
    EXPECT_EQ(5, trackResult.erasedAt(5, 4));
}

TEST(RensaChainTrackerTest, rensaChainTracker)
{
    CoreField cf(
        "..BB.."
        "RRRRBY"
    );

    RensaChainTracker tracker;
    RensaResult rensaResult = cf.simulate(&tracker);

    const RensaChainTrackResult& trackResult = tracker.result();

    EXPECT_EQ(1, rensaResult.chains);
    EXPECT_EQ(1, trackResult.erasedAt(1, 1));
    EXPECT_EQ(1, trackResult.erasedAt(2, 1));
    EXPECT_EQ(1, trackResult.erasedAt(3, 1));
    EXPECT_EQ(1, trackResult.erasedAt(4, 1));
    EXPECT_EQ(0, trackResult.erasedAt(1, 2));
}
