#include "gui/box.h"

#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <vector>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

TEST(BoxTest, ToSDLRect)
{
   Box b(10, 20, 40, 60);
   SDL_Rect r = b.toSDLRect();

   EXPECT_EQ(10, r.x);
   EXPECT_EQ(20, r.y);
   EXPECT_EQ(30, r.w);
   EXPECT_EQ(40, r.h);
}
