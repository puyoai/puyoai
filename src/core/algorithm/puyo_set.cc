#include "core/algorithm/puyo_set.h"

#include "core/algorithm/column_puyo_list.h"

using namespace std;

void PuyoSet::add(const ColumnPuyoList& list)
{
    for (const auto& p : list.list()) {
        add(get<1>(p), get<0>(p));
    }
}
