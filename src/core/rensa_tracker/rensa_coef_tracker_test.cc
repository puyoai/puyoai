#include "core/rensa_tracker/rensa_coef_tracker.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

TEST(RensaCoefTrackResult, score)
{
    RensaCoefResult result;
    result.setCoef(1, 4, 0, 0);
    result.setCoef(2, 4, 0, 0);
    result.setCoef(3, 4, 0, 0);

    EXPECT_EQ(40 * (1 + 8 + 16), result.score(0));
    EXPECT_EQ(40 * (1 + 8 + 16 + 32 + 64), result.score(2));
}

TEST(RensaCoefTrackerTest, simualteWithRensaCoefResult)
{
    CoreField cf("R...RR"
                 "RGBRYR"
                 "RRGBBY"
                 "GGBYYR");

    RensaCoefTracker tracker;
    RensaResult rensaResult = cf.simulate(&tracker);

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
