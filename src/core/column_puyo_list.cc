#include "core/column_puyo_list.h"

#include <sstream>

using namespace std;

string ColumnPuyoList::toString() const
{
    ostringstream oss;
    for (const auto& p : *this) {
        oss << '(' << p.x << ',' << toChar(p.color) << ')';
    }

    return oss.str();
}

// static
bool operator==(const ColumnPuyoList& lhs, const ColumnPuyoList& rhs)
{
    if (lhs.size_ != rhs.size_)
        return false;

    for (int i = 0; i < lhs.size_; ++i) {
        if (lhs.puyos_[i] != rhs.puyos_[i])
                return false;
    }
    return true;
}
