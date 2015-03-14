#ifndef CORE_COLUMN_PUYO_LIST_H_
#define CORE_COLUMN_PUYO_LIST_H_

#include <array>
#include <iterator>
#include <string>

#include <glog/logging.h>

#include "column_puyo.h"

class ColumnPuyoList {
public:
    static const int MAX_SIZE = 16;

    ColumnPuyoList() {}

    ColumnPuyoList(int x, PuyoColor c, int n)
    {
        for (int i = 0; i < n; ++i)
            add(x, c);
    }

    bool isEmpty() const { return size_ == 0; }
    int size() const { return size_; }

    bool add(int x, PuyoColor c) { return add(ColumnPuyo(x, c)); }
    bool add(const ColumnPuyo& cp)
    {
        if (MAX_SIZE <= size())
            return false;

        puyos_[size_++] = cp;
        return true;
    }

    // Appends |cpl|. If the result size exceeds the max size, false will be returned.
    bool append(const ColumnPuyoList& cpl)
    {
        if (size_ + cpl.size() > MAX_SIZE)
            return false;
        for (const auto& cp : cpl)
            add(cp.x, cp.color);
        return true;
    }

    void clear() { size_ = 0; }

    void removeLastAddedPuyo()
    {
        DCHECK(0 < size_);
        --size_;
    }

    std::array<ColumnPuyo, MAX_SIZE>::const_iterator begin() const { return std::begin(puyos_); }
    std::array<ColumnPuyo, MAX_SIZE>::const_iterator end() const { return std::begin(puyos_) + size_; }

    std::string toString() const;

    friend bool operator==(const ColumnPuyoList&, const ColumnPuyoList&);

private:
    // We don't make this std::vector due to performance reason.
    int size_ = 0;
    std::array<ColumnPuyo, MAX_SIZE> puyos_;
};

#endif
