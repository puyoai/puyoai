#ifndef CORE_COLUMN_PUYO_H_
#define CORE_COLUMN_PUYO_H_

#include "core/puyo_color.h"

struct ColumnPuyo {
    ColumnPuyo() {}
    ColumnPuyo(int x, PuyoColor color) : x(x), color(color) {}

    friend bool operator==(const ColumnPuyo& lhs, const ColumnPuyo& rhs)
    {
        return lhs.x == rhs.x && lhs.color == rhs.color;
    }

    friend bool operator!=(const ColumnPuyo& lhs, const ColumnPuyo& rhs) { return !(lhs == rhs); }

    int x;
    PuyoColor color;
};

#endif
