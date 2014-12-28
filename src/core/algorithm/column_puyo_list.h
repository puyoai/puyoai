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

    friend bool operator==(const ColumnPuyo& lhs, const ColumnPuyo& rhs)
    {
        return lhs.x == rhs.x && lhs.color == rhs.color;
    }

    friend bool operator!=(const ColumnPuyo& lhs, const ColumnPuyo& rhs) { return !(lhs == rhs); }
};

class ColumnPuyoList {
public:
    static const int MAX_SIZE = 8;

    ColumnPuyoList() {}

    ColumnPuyoList(int x, PuyoColor c, int n)
    {
        addPuyo(x, c, n);
    }

    bool isEmpty() const { return size_ == 0; }
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

    // Appends |cpl|. If the result size exceeds the max size, false will be returned.
    bool append(const ColumnPuyoList& cpl)
    {
        if (size_ + cpl.size() > MAX_SIZE)
            return false;
        for (const auto& cp : cpl)
            addPuyo(cp.x, cp.color);
        return true;
    }

    void clear()
    {
        size_ = 0;
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

    friend bool operator==(const ColumnPuyoList& lhs, const ColumnPuyoList& rhs)
    {
        if (lhs.size_ != rhs.size_)
            return false;

        for (int i = 0; i < lhs.size_; ++i) {
            if (lhs.puyos_[i] != rhs.puyos_[i])
                return false;
        }
        return true;
    }

private:
    int size_ = 0;
    std::array<ColumnPuyo, MAX_SIZE> puyos_;
};

#endif
