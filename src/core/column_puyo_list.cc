#include "core/column_puyo_list.h"

#include <sstream>

#include "core/column_puyo.h"

using namespace std;

string ColumnPuyoList::toString() const
{
    ostringstream oss;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < size_[i]; ++j) {
            oss << '(' << (i + 1) << toChar(puyos_[i][j]) << ')';
        }
    }

    return oss.str();
}

// static
bool operator==(const ColumnPuyoList& lhs, const ColumnPuyoList& rhs)
{
    for (int i = 0; i < 6; ++i) {
        if (lhs.size_[i] != rhs.size_[i])
            return false;
    }

    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < lhs.size_[i]; ++j) {
            if (lhs.puyos_[i][j] != rhs.puyos_[i][j])
                return false;
        }
    }
    return true;
}
