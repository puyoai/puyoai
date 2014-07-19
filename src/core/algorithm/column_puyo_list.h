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

    ColumnPuyoList() {}

    ColumnPuyoList(int x, PuyoColor c, int n)
    {
        addPuyo(x, c, n);
    }

    int size() const { return size_; }

    void addPuyo(int x, PuyoColor c)
    {
        DCHECK(size_ < MAX_SIZE);
        puyos_[size_++] = ColumnPuyo { x, c };
    }

    void addPuyo(int x, PuyoColor c, int n)
    {
        for (int i = 0; i < n; ++i) {
            addPuyo(x, c);
        }
    }

    void append(const ColumnPuyoList& cpl)
    {
        for (const auto& cp : cpl)
            addPuyo(cp.x, cp.color);
    }

    void removeLastAddedPuyo()
    {
        DCHECK(0 < size_);
        --size_;
    }

    std::array<ColumnPuyo, MAX_SIZE>::const_iterator begin() const
    {
        return std::begin(puyos_);
    }

    std::array<ColumnPuyo, MAX_SIZE>::const_iterator end() const
    {
        return std::begin(puyos_) + size_;
    }

    PuyoSet toPuyoSet() const;
    std::string toString() const;

private:
    int size_ = 0;
    std::array<ColumnPuyo, MAX_SIZE> puyos_;
};

#endif
