#include "core/rensa_tracker/rensa_last_vanished_position_tracker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

TEST(RensaLastVanishedPositionTrackerTest, simualteWithLastVanishedPositionTracker)
{
    CoreField f(
        "..YY.."
        "..GGY."
        "RRRRGG");

    FieldBits expected(
        "..1111");

    RensaLastVanishedPositionTracker tracker;

    f.simulate(&tracker);
    EXPECT_EQ(expected, tracker.result().lastVanishedPositionBits());
}
