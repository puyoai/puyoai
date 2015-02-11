#include "core/frame_request.h"

#include <string>
#include <map>

#include <gtest/gtest.h>

using namespace std;

TEST(FrameRequestTest, parse)
{
    FrameRequest request = FrameRequest::parse("ID=1");

    EXPECT_EQ(1, request.frameId);
    EXPECT_EQ(GameResult::PLAYING, request.gameResult);

    EXPECT_TRUE(request.isValid());
    EXPECT_FALSE(request.hasGameEnd());
}
