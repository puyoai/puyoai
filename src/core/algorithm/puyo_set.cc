#include "core/algorithm/puyo_set.h"

#include "core/column_puyo_list.h"
#include "core/column_puyo.h"

using namespace std;

void PuyoSet::add(const ColumnPuyoList& list)
{
    for (const auto& p : list) {
        add(p.color);
    }
}
