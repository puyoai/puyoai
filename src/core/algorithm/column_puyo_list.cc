#include "core/algorithm/column_puyo_list.h"

#include <sstream>

using namespace std;

string ColumnPuyoList::toString() const
{
    ostringstream oss;
    for (const auto& p : puyos_) {
        oss << '(' << p.x << ',' << p.color << ')';
    }

    return oss.str();
}

PuyoSet ColumnPuyoList::toPuyoSet() const
{
    PuyoSet puyoSet;
    for (const auto& p : puyos_) {
        puyoSet.add(p.color);
    }

    return puyoSet;
}
