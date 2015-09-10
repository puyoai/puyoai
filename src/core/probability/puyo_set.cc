#include "core/probability/puyo_set.h"

#include "core/column_puyo_list.h"
#include "core/column_puyo.h"

using namespace std;

void PuyoSet::add(const ColumnPuyoList& cpl)
{
    for (int x = 1; x <= 6; ++x) {
        int h = cpl.sizeOn(x);
        for (int i = 0; i < h; ++i)
            add(cpl.get(x, i));
    }
}
