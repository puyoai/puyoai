#include "template.h"

#include "core/puyo_color.h"
#include "lockit_constant.h"

namespace test_lockit {

int gtr(const PuyoColor field[][kHeight])
{
    int score = 0;

    static const struct {
        int x0, y0, x1, y1;
    } expect_same[] = {
        {0, 0, 1, 0}, {0, 0, 1, 2}, {0, 0, 2, 1},
        {1, 0, 1, 2}, {1, 0, 2, 1}, {1, 2, 2, 1},
        {0, 1, 0, 2}, {0, 1, 1, 1}, {1, 1, 0, 2},
    };
    for (const auto& same : expect_same) {
        PuyoColor c0 = field[same.x0][same.y0];
        PuyoColor c1 = field[same.x1][same.y1];
        if (c0 == PuyoColor::EMPTY || c1 == PuyoColor::EMPTY)
            continue;
        if (c0 == c1)
            score += 1000;
        else
            score -= 1000;
    }

    static const struct {
        int x0, y0, x1, y1;
        int penalty;
    } expect_diff[] = {
        {0, 2, 0, 3, 2000},
        {1, 2, 1, 3, 500}, {1, 2, 2, 2, 500}, {2, 1, 2, 2, 500},
        {1, 0, 2, 0, 1000}, {2, 1, 2, 0, 1000},
    };
    for (const auto& diff : expect_diff) {
        PuyoColor c0 = field[diff.x0][diff.y0];
        PuyoColor c1 = field[diff.x1][diff.y1];
        if (c0 != PuyoColor::EMPTY && c0 == c1)
            score -= diff.penalty;
    }

    return score;
}

}  // namespace test_lockit
