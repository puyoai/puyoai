#include "core/algorithm/puyo_set.h"

#include "core/algorithm/column_puyo_list.h"

using namespace std;

void PuyoSet::add(const ColumnPuyoList& list)
{
    for (const auto& p : list) {
        add(p.color);
    }
}
