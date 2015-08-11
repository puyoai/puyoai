#include "core/rensa_tracker/rensa_tracker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

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

TEST(RensaCoefTrackerTest, simualteWithRensaCoefResult)
{
    CoreField f("R...RR"
                "RGBRYR"
                "RRGBBY"
                "GGBYYR");

    RensaCoefTracker tracker;
    RensaResult rensaResult = f.simulate(&tracker);

    const RensaCoefResult& coefResult = tracker.result();

    EXPECT_EQ(5, rensaResult.chains);
    EXPECT_EQ(4, coefResult.numErased(1));
    EXPECT_EQ(4, coefResult.numErased(2));
    EXPECT_EQ(4, coefResult.numErased(3));
    EXPECT_EQ(4, coefResult.numErased(4));
    EXPECT_EQ(5, coefResult.numErased(5));

    EXPECT_EQ(1, coefResult.coef(1));
    EXPECT_EQ(8, coefResult.coef(2));
    EXPECT_EQ(16, coefResult.coef(3));
    EXPECT_EQ(32, coefResult.coef(4));
    EXPECT_EQ(64 + 2, coefResult.coef(5));
}

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

TEST(CompositeRensaTracker, track)
{
    const CoreField original(
        ".RBGY."
        "RBGYR."
        "RBGYR."
        "RBGYRR");

    RensaCoefTracker coefTracker1, coefTracker2;
    RensaChainTracker chainTracker1, chainTracker2;
    CompositeRensaTracker<RensaCoefTracker, RensaChainTracker> tracker(&coefTracker2, &chainTracker2);

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
