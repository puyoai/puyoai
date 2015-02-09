#include "base/strings.h"

#include <gtest/gtest.h>

TEST(StringsTest, contains)
{
    EXPECT_TRUE(strings::contains("foo", "oo"));
    EXPECT_TRUE(strings::contains("foo", "fo"));
    EXPECT_TRUE(strings::contains("foo", "o"));
    EXPECT_TRUE(strings::contains("foo", "f"));

    EXPECT_FALSE(strings::contains("foo", "fooo"));
    EXPECT_FALSE(strings::contains("foo", "ooo"));
    EXPECT_FALSE(strings::contains("foo", "g"));
    EXPECT_FALSE(strings::contains("foo", "hoge"));
}
