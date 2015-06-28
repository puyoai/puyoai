#include "core/rensa_result.h"

#include <sstream>

#include <gtest/gtest.h>

#include "core/score.h"

using namespace std;

TEST(RensaResultTest, ignitionRensaResult)
{
    RensaResult rensaResult(2, ACCUMULATED_RENSA_SCORE[2], 140, false);
    IgnitionRensaResult ignitionRensaResult(rensaResult, 40, 36);

    EXPECT_EQ(140 + 40 + 36, ignitionRensaResult.totalFrames());
}
