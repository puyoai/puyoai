#ifndef CORE_ALGORITHM_COLUMN_PUYO_LIST_H_
#define CORE_ALGORITHM_COLUMN_PUYO_LIST_H_

#include <array>
#include <iterator>
#include <string>

#include <glog/logging.h>

#include "core/algorithm/puyo_set.h"
#include "core/puyo_color.h"

struct ColumnPuyo {
    int x;
    PuyoColor color;
};

class ColumnPuyoList {
public:
    static const int MAX_SIZE = 8;

    void addPuyo(int x, PuyoColor c)
    {
        DCHECK(size < MAX_SIZE);
        puyos_[size++] = ColumnPuyo { x, c };
    }

    void removeLastAddedPuyo()
    {
        DCHECK(0 < size);
        --size;
    }

    std::array<ColumnPuyo, MAX_SIZE>::const_iterator begin() const {
        return std::begin(puyos_);
    }

    std::array<ColumnPuyo, MAX_SIZE>::const_iterator end() const {
        return std::begin(puyos_) + size;
    }

    PuyoSet toPuyoSet() const;
    std::string toString() const;

private:
    int size = 0;
    std::array<ColumnPuyo, MAX_SIZE> puyos_;
};

#endif
