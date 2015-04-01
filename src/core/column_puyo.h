#ifndef CORE_COLUMN_PUYO_H_
#define CORE_COLUMN_PUYO_H_

#include "core/puyo_color.h"

// ColumnPuyo is a pair of x-axis and PuyoColor.
struct ColumnPuyo {
    ColumnPuyo() {}
    ColumnPuyo(int x, PuyoColor color) : x(x), color(color) {}

    bool isValid() const { return x > 0; }

    friend bool operator==(const ColumnPuyo& lhs, const ColumnPuyo& rhs)
    {
        return lhs.x == rhs.x && lhs.color == rhs.color;
    }

    friend bool operator!=(const ColumnPuyo& lhs, const ColumnPuyo& rhs) { return !(lhs == rhs); }

    int x = 0;
    PuyoColor color = PuyoColor::EMPTY;
};

#endif
