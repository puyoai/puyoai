#include "core/rensa_tracker/rensa_vanishing_position_tracker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

TEST(RensaVanishingPositionTrackerTest, simualteWithRensaVanishingPositionResult)
{
    CoreField f("R....."
                "RG...."
                "BB...."
                "YYYYRR"
                "BBRBBR"
                "RRGRRB"
                "GGRBBR");

    RensaVanishingPositionTracker tracker;
    RensaResult rensaResult = f.simulate(&tracker);

    const RensaVanishingPositionResult& positionResult = tracker.result();

    EXPECT_EQ(7, rensaResult.chains);
    EXPECT_EQ(7, positionResult.size());
    EXPECT_EQ(0U, positionResult.getReferenceFallingPuyosAt(1).size());
    EXPECT_EQ(4U, positionResult.getReferenceBasePuyosAt(1).size());
    EXPECT_EQ(2U, positionResult.getReferenceFallingPuyosAt(2).size());
    EXPECT_EQ(2U, positionResult.getReferenceBasePuyosAt(2).size());
    EXPECT_EQ(2U, positionResult.getReferenceFallingPuyosAt(3).size());
    EXPECT_EQ(2U, positionResult.getReferenceBasePuyosAt(3).size());
    EXPECT_EQ(1U, positionResult.getReferenceFallingPuyosAt(4).size());
    EXPECT_EQ(3U, positionResult.getReferenceBasePuyosAt(4).size());
    EXPECT_EQ(2, positionResult.getReferenceFallingPuyosAt(4)[0].x);
    EXPECT_EQ(3, positionResult.getReferenceFallingPuyosAt(4)[0].y);
}
