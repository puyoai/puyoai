#include "core/algorithm/column_puyo_list.h"

#include <sstream>

using namespace std;

string ColumnPuyoList::toString() const
{
    ostringstream oss;
    for (auto p : puyo_) {
        oss << '(' << get<0>(p) << ',' << charOfPuyoColor(get<1>(p)) << ')';
    }

    return oss.str();
}

PuyoSet ColumnPuyoList::toPuyoSet() const
{
    PuyoSet puyoSet;
    for (const auto& p : list()) {
        puyoSet.add(get<1>(p));
    }

    return puyoSet;
}
