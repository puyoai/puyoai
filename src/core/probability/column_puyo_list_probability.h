#ifndef CORE_PROBABILITY_COLUMN_PUYO_LIST_PROBABILITY_H_
#define CORE_PROBABILITY_COLUMN_PUYO_LIST_PROBABILITY_H_

#include <memory>
#include <unordered_map>

#include "base/noncopyable.h"
#include "core/column_puyo_list.h"

class ColumnPuyoListProbability : noncopyable, nonmovable {
public:
    // Taking ColumnPuyoListProbability instance. This might be slow.
    static const ColumnPuyoListProbability* instanceSlow();

    // Returns the expected numbef of kumipuyos to fill ColumnPuyoList.
    // NOTE: currently this is not thread-safe.
    double necessaryKumipuyos(const ColumnPuyoList&) const;

private:
    ColumnPuyoListProbability();

    std::unordered_map<ColumnPuyoList, double> m_;
};

#endif // CORE_PROBABILITY_COLUMN_PUYO_LIST_PROBABILITY_H_
