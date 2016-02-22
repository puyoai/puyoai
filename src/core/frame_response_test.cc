#include "core/frame_response.h"

#include <string>

#include <gtest/gtest.h>

using namespace std;

TEST(FrameResponseTest, parse)
{
    std::string line = "ID=1 X=3 R=0 MSG=hoge,fuga";
    FrameResponse response = FrameResponse::parsePayload(line.data(), line.size());

    EXPECT_EQ(1, response.frameId);
    EXPECT_TRUE(response.decision.isValid());

    EXPECT_EQ(Decision(3, 0), response.decision);
    EXPECT_EQ("hoge\nfuga", response.message);

    EXPECT_TRUE(response.isValid());
}

TEST(FrameResponseTest, toStringAndParse)
{
    FrameResponse expected;
    expected.frameId = 100;
    expected.decision = Decision(3, 0);
    expected.message = "message with space (1)\nmessage with space (2)";

    std::string line = expected.toString();
    FrameResponse actual = FrameResponse::parsePayload(line.data(), line.size());

    EXPECT_TRUE(actual.isValid());
    EXPECT_EQ(expected.frameId, actual.frameId);
    EXPECT_EQ(expected.decision, actual.decision);
    EXPECT_EQ(expected.message, actual.message);
}
