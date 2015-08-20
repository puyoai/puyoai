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

TEST(RensaChainTrackerTest, simulate1)
{
    CoreField cf("R...R."
                 "RBYRGR"
                 "RRBYYG"
                 "BBYGGR");

    RensaChainTrackResult expected(
        "1...5."
        "123545"
        "112334"
        "223445");

    RensaChainTracker tracker;
    RensaResult rensaResult = cf.simulate(&tracker);
    EXPECT_EQ(5, rensaResult.chains);

    const RensaChainTrackResult& trackResult = tracker.result();
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            EXPECT_EQ(expected.erasedAt(x, y), trackResult.erasedAt(x, y)) << "x=" << x << " y=" << y;
        }
    }
}

TEST(RensaChainTrackerTest, simulate2)
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
    EXPECT_EQ(0, trackResult.erasedAt(2, 2));
    EXPECT_EQ(0, trackResult.erasedAt(3, 2));
    EXPECT_EQ(0, trackResult.erasedAt(4, 2));
    EXPECT_EQ(0, trackResult.erasedAt(6, 1));
}
