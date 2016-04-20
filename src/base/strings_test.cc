#include "base/strings.h"

#include <gtest/gtest.h>

using namespace std;

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

TEST(StringsTest, hasPrefix)
{
    EXPECT_TRUE(strings::hasPrefix("foo", ""));
    EXPECT_TRUE(strings::hasPrefix("foo", "f"));
    EXPECT_TRUE(strings::hasPrefix("foo", "fo"));
    EXPECT_TRUE(strings::hasPrefix("foo", "foo"));

    EXPECT_FALSE(strings::hasPrefix("foo", "o"));
    EXPECT_FALSE(strings::hasPrefix("foo", "oo"));
    EXPECT_FALSE(strings::hasPrefix("foo", "hoge"));
    EXPECT_FALSE(strings::hasPrefix("foo", "barfoo"));
}

TEST(StringsTest, hasSuffix)
{
    EXPECT_TRUE(strings::hasSuffix("foo", ""));
    EXPECT_TRUE(strings::hasSuffix("foo", "o"));
    EXPECT_TRUE(strings::hasSuffix("foo", "oo"));
    EXPECT_TRUE(strings::hasSuffix("foo", "foo"));

    EXPECT_FALSE(strings::hasSuffix("foo", "f"));
    EXPECT_FALSE(strings::hasSuffix("foo", "fo"));
    EXPECT_FALSE(strings::hasSuffix("foo", "hoge"));
    EXPECT_FALSE(strings::hasSuffix("foo", "barfoo"));
}

TEST(StringsTest, join)
{
    vector<string> ss {
        "foo", "bar", "foobar"
    };

    EXPECT_EQ("foo,bar,foobar", strings::join(ss, ","));
}

TEST(StringsTest, isAllDigits)
{
    EXPECT_TRUE(strings::isAllDigits(""));
    EXPECT_TRUE(strings::isAllDigits("0"));
    EXPECT_TRUE(strings::isAllDigits("0123456789"));

    EXPECT_FALSE(strings::isAllDigits("abc"));
    EXPECT_FALSE(strings::isAllDigits("+0"));
    EXPECT_FALSE(strings::isAllDigits("-12"));

    EXPECT_FALSE(strings::isAllDigits(" \t\r\n"));
    EXPECT_FALSE(strings::isAllDigits("/@"));
}
