#ifndef CORE_ALGORITHM_COLUMN_PUYO_LIST_H_
#define CORE_ALGORITHM_COLUMN_PUYO_LIST_H_

#include <string>
#include <tuple>
#include <vector>

#include "core/algorithm/puyo_set.h"
#include "core/puyo_color.h"

class ColumnPuyoList {
public:
    void addPuyo(int x, PuyoColor c)
    {
        puyo_.push_back(std::make_tuple(x, c));
    }

    void removeLastAddedPuyo()
    {
        DCHECK(!puyo_.empty());
        puyo_.pop_back();
    }

    const std::vector<std::tuple<int, PuyoColor>>& list() const {
        return puyo_;
    }

    PuyoSet toPuyoSet() const;

    std::string toString() const;

private:
    std::vector<std::tuple<int, PuyoColor>> puyo_;
};

#endif
