#include "capture/real_color_field.h"

#include <gtest/gtest.h>

TEST(RealColorFieldTest, empty)
{
    RealColorField f;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            EXPECT_EQ(RealColor::RC_EMPTY, f.get(x, y));
        }
    }
}

TEST(RealColorFieldTest, setget)
{
    RealColorField f;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            f.set(x, y, RealColor::RC_OJAMA);
            EXPECT_EQ(RealColor::RC_OJAMA, f.get(x, y));
            f.set(x, y, RealColor::RC_EMPTY);
            EXPECT_EQ(RealColor::RC_EMPTY, f.get(x, y));
        }
    }
}
