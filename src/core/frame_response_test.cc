#include "core/frame_response.h"

#include <string>

#include <gtest/gtest.h>

using namespace std;

TEST(FrameResponseTest, parse)
{
    FrameResponse response = FrameResponse::parse("ID=1 X=3 R=0 MSG=hoge,fuga");

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

    FrameResponse actual = FrameResponse::parse(expected.toString());

    EXPECT_TRUE(actual.isValid());
    EXPECT_EQ(expected.frameId, actual.frameId);
    EXPECT_EQ(expected.decision, actual.decision);
    EXPECT_EQ(expected.message, actual.message);
}
