#include "base/file.h"

#include <gtest/gtest.h>

TEST(FileTest, joinPath)
{
    EXPECT_EQ("/", file::joinPath("/", "/"));
    EXPECT_EQ("/ab/cd", file::joinPath("/ab", "/cd"));
    EXPECT_EQ("/cd", file::joinPath("/", "cd"));
    EXPECT_EQ("/ab/cd", file::joinPath("/ab/", "/cd"));
    EXPECT_EQ("ab/cd", file::joinPath("ab", "/cd"));
}
