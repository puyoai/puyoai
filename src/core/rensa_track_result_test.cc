#include "core/rensa_track_result.h"

#include <gtest/gtest.h>

using namespace std;

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

TEST(RensaCoefTrackResult, score)
{
    RensaCoefResult result;
    result.setCoef(1, 4, 0, 0);
    result.setCoef(2, 4, 0, 0);
    result.setCoef(3, 4, 0, 0);

    EXPECT_EQ(40 * (1 + 8 + 16), result.score(0));
    EXPECT_EQ(40 * (1 + 8 + 16 + 32 + 64), result.score(2));
}
