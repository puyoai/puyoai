#ifndef CORE_FIELD_FIELD_BIT_FIELD_H_
#define CORE_FIELD_FIELD_BIT_FIELD_H_

#include <bitset>
#include <glog/logging.h>

#include "core/field_constant.h"

// FieldBitField is a bitset whose size if the same as field.
class FieldBitField {
public:
    bool get(int x, int y) const { return field_[index(x, y)]; }
    void set(int x, int y, bool flag = true) { field_.set(index(x, y), flag); }
    void clear(int x, int y) { field_.set(index(x, y), false); }

private:
    int index(int x, int y) const
    {
        DCHECK(0 <= x && x < FieldConstant::MAP_WIDTH);
        DCHECK(0 <= y && y < FieldConstant::MAP_HEIGHT);
        return y * FieldConstant::MAP_WIDTH + x;
    }

    std::bitset<FieldConstant::MAP_HEIGHT * FieldConstant::MAP_WIDTH> field_;
};

#endif
