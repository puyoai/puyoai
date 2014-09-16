#include "core/plain_field.h"

#include <gtest/gtest.h>

using namespace std;

TEST(PlainFieldTest, isVisibllySame)
{
    PlainField f1;
    PlainField f2(
        "OOOOOO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    PlainField f3(
        "OOOOOO"
        "OOOOOO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    PlainField f4(
        "OO OOO"
        "OO OOO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_TRUE(f1.isVisibllySame(f1));
    EXPECT_TRUE(f2.isVisibllySame(f3));
    EXPECT_FALSE(f3.isVisibllySame(f4));
}

TEST(PlainFieldTest, canBeOverriden)
{
    PlainField f1(
        "RRRGGG");
    PlainField f2(
        "   OOO"
        "RRRGGG");
    PlainField f3(
        "OOO"
        "RRRGGG");
    PlainField f4(
        "OOOOOO"
        "RRRGGG");
    PlainField f5(
        "OOOOOO"
        "OOOOOO"
        "RRRGGG");

    EXPECT_TRUE(f1.canBeOverriden(f2));
    EXPECT_TRUE(f1.canBeOverriden(f3));
    EXPECT_TRUE(f1.canBeOverriden(f4));
    EXPECT_FALSE(f1.canBeOverriden(f5));
    EXPECT_TRUE(f2.canBeOverriden(f3));
    EXPECT_TRUE(f2.canBeOverriden(f4));
    EXPECT_FALSE(f2.canBeOverriden(f5));
    EXPECT_TRUE(f3.canBeOverriden(f4));
    EXPECT_FALSE(f3.canBeOverriden(f5));
    EXPECT_TRUE(f4.canBeOverriden(f5));
}
