#include "core/frame_response.h"

#include <string>
#include <map>

#include <gtest/gtest.h>

using namespace std;

TEST(FrameResponseTest, parse)
{
    FrameResponse response = FrameResponse::parse("ID=1 X=3 R=0 MSG=hoge");

    EXPECT_FALSE(response.connectionLost);
    EXPECT_TRUE(response.received);
    EXPECT_EQ(1, response.frameId);
    EXPECT_TRUE(response.decision.isValid());

    EXPECT_EQ(Decision(3, 0), response.decision);
    EXPECT_EQ("hoge", response.msg);

    EXPECT_TRUE(response.isValid());
}
