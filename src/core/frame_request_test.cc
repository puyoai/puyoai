#include "core/frame_request.h"

#include <gtest/gtest.h>

using namespace std;

TEST(FrameRequestTest, parse)
{
    std::string line = "ID=1";
    FrameRequest request = FrameRequest::parsePayload(line.data(), line.size());

    EXPECT_EQ(1, request.frameId);
    EXPECT_EQ(GameResult::PLAYING, request.gameResult);

    EXPECT_TRUE(request.isValid());
    EXPECT_FALSE(request.hasGameEnd());
}

TEST(FrameRequestTest, parseMatchEnd_true)
{
    std::string line = "ID=1 MATCHEND=1";
    FrameRequest request = FrameRequest::parsePayload(line.data(), line.size());

    EXPECT_TRUE(request.matchEnd);
}

TEST(FrameRequestTest, parseMatchEnd_false1)
{
    std::string line = "ID=1";
    FrameRequest request = FrameRequest::parsePayload(line.data(), line.size());

    EXPECT_FALSE(request.matchEnd);
}

TEST(FrameRequestTest, parseMatchEnd_false2)
{
    std::string line = "ID=1 MATCHEND=0";
    FrameRequest request = FrameRequest::parsePayload(line.data(), line.size());

    EXPECT_FALSE(request.matchEnd);
}
