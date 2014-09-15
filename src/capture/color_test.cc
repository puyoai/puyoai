#include "capture/color.h"

#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>

using namespace std;

TEST(RGB2HSV, ForAll)
{
    for (int r = 0; r < 256; ++r) {
        for (int g = 0; g < 256; ++g) {
            for (int b = 0; b < 256; ++b) {
                RGB rgb1(r, g, b);
                HSV hsv = rgb1.toHSV();
                RGB rgb2 = hsv.toRGB();
                EXPECT_TRUE(abs(rgb1.r - rgb2.r) <= 1 && abs(rgb1.g - rgb2.g) <= 1 && abs(rgb1.b - rgb2.b) <= 1);
            }
        }
    }
}

