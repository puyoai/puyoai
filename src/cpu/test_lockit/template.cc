#include "template.h"

#include "color.h"

namespace test_lockit {

int gtr(const TLColor field[][kHeight])
{
    int score = 0;

    int expect_same[][4] = {
        {0, 0, 1, 0}, {0, 0, 1, 2}, {0, 0, 2, 1},
        {1, 0, 1, 2}, {1, 0, 2, 1}, {1, 2, 2, 1},
        {0, 1, 0, 2}, {0, 1, 1, 1}, {1, 1, 0, 2},
    };
    for (auto& xy : expect_same) {
        int x0 = xy[0], y0 = xy[1];
        int x1 = xy[2], y1 = xy[3];
        if (field[x0][y0] == TLColor::EMPTY || field[x1][y1] == TLColor::EMPTY)
            continue;
        if (field[x0][y0] == field[x1][y1])
            score += 1000;
        else
            score -= 1000;
    }

    int expect_diff[][5] = {
        {0, 2, 0, 3, 2000},
        {1, 2, 1, 3, 500}, {1, 2, 2, 2, 500}, {2, 1, 2, 2, 500},
        {1, 0, 2, 0, 1000}, {2, 1, 2, 0, 1000},
    };
    for (auto& xy : expect_diff) {
        int x0 = xy[0], y0 = xy[1];
        int x1 = xy[2], y1 = xy[3];
        int penalty = xy[4];

        if (field[x0][y0] == TLColor::EMPTY || field[x1][y1] == TLColor::EMPTY)
            continue;
        if (field[x0][y0] == field[x1][y1])
            score -= penalty;
    }

    return score;
}

}  // namespace test_lockit
