#include "core/rensa_tracker/rensa_existing_position_tracker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

TEST(RensaExistingPositionTrackerTest, simualteWithRensaExistingPositionTracker)
{
    CoreField f(
        "..YY.."
        "..GGY."
        "RRRRGG");

    FieldBits bits(
        "..11.."
        "..11.."
        "...111");

    FieldBits expected1(
        "..11.."
        "..1111");

    FieldBits expected2(
        "..11..");

    RensaExistingPositionTracker tracker(bits);

    f.vanishDrop(&tracker);
    EXPECT_EQ(expected1, tracker.result().existingBits());

    f.vanishDrop(&tracker);
    EXPECT_EQ(expected2, tracker.result().existingBits());
}
