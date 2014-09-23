#include "core/server/commentator.h"

#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <vector>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "base/base.h"

using namespace std;

class CommentatorTest : public testing::Test {
};

#if 0
TEST_F(CommentatorTest, getPotentialMaxChain)
{
    FieldWithColorSequence f("600000"
                             "500000"
                             "500000"
                             "440400"
                             "467564"
                             "557664"
                             "666776"
                             "575664"
                             "554644"
                             "445765"
                             "477655"
                             "555766");

    int heights[6];
    CLEAR_ARRAY(heights);
    for (int x = 0; x < 6; x++) {
        int h = 1;
        while (h <= 13 && f.Get(x+1, h) != EMPTY) {
            h++;
        }
        heights[x] = h;
    }
    float best_score = 100;
    vector<vector<PuyoColor> > csp(6, vector<PuyoColor>());
    vector<vector<PuyoColor> > best_csp(6, vector<PuyoColor>());
    Commentator::getPotentialMaxChain(f, 0, 4, &csp, heights,
                                      &best_score, &best_csp);
    EXPECT_EQ(0UL, best_csp[0].size());
    ASSERT_EQ(1UL, best_csp[1].size());
    EXPECT_EQ(RED, RED + best_csp[1][0]);
    EXPECT_EQ(0UL, best_csp[2].size());
    EXPECT_EQ(0UL, best_csp[3].size());
    EXPECT_EQ(0UL, best_csp[4].size());
    ASSERT_EQ(1UL, best_csp[5].size());
    EXPECT_EQ(BLUE, RED + best_csp[5][0]);
}
#endif
