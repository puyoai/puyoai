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

TEST(StringsTest, isSuffix)
{
    EXPECT_TRUE(strings::isSuffix("foo", ""));
    EXPECT_TRUE(strings::isSuffix("foo", "o"));
    EXPECT_TRUE(strings::isSuffix("foo", "oo"));
    EXPECT_TRUE(strings::isSuffix("foo", "foo"));

    EXPECT_FALSE(strings::isSuffix("foo", "f"));
    EXPECT_FALSE(strings::isSuffix("foo", "fo"));
    EXPECT_FALSE(strings::isSuffix("foo", "hoge"));
    EXPECT_FALSE(strings::isSuffix("foo", "barfoo"));
}
