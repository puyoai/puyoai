#ifndef CORE_FIELD_BIT_FIELD_H_
#define CORE_FIELD_BIT_FIELD_H_

#include <glog/logging.h>

#include "core/field_constant.h"

// FieldBitField is a bitset whose size if the same as field.
class FieldBitField {
public:
    FieldBitField() : field_{} {}

    bool get(int x, int y) const { return field_[index(x, y)]; }
    void set(int x, int y, bool flag = true) { field_[index(x, y)] = flag; }
    void clear(int x, int y) { field_[index(x, y)] = false; }

    bool operator()(int x, int y) const { return get(x, y); }

private:
    int index(int x, int y) const
    {
        DCHECK(0 <= x && x < FieldConstant::MAP_WIDTH);
        DCHECK(0 <= y && y < FieldConstant::MAP_HEIGHT);
        return y * FieldConstant::MAP_WIDTH + x;
    }

    bool field_[FieldConstant::MAP_HEIGHT * FieldConstant::MAP_WIDTH];
};

#endif
