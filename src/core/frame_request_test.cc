#include "core/frame_request.h"

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

TEST(FrameRequestTest, parseMatchEnd_true)
{
    FrameRequest request = FrameRequest::parse("ID=1 MATCHEND=1");
    EXPECT_TRUE(request.matchEnd);
}

TEST(FrameRequestTest, parseMatchEnd_false1)
{
    FrameRequest request = FrameRequest::parse("ID=1");
    EXPECT_FALSE(request.matchEnd);
}

TEST(FrameRequestTest, parseMatchEnd_false2)
{
    FrameRequest request = FrameRequest::parse("ID=1 MATCHEND=0");
    EXPECT_FALSE(request.matchEnd);
}
