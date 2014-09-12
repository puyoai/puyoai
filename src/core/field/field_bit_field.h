#ifndef CORE_FIELD_FIELD_BIT_FIELD_H_
#define CORE_FIELD_FIELD_BIT_FIELD_H_

#include <bitset>
#include <glog/logging.h>

#include "core/plain_field.h"

// FieldBitField is a bitset whose size if the same as field.
class FieldBitField {
public:
    bool get(int x, int y) const { return field_[index(x, y)]; }
    void set(int x, int y) { field_.set(index(x, y)); }
    void clear(int x, int y) { field_.set(index(x, y), false); }

private:
    int index(int x, int y) const
    {
        DCHECK(0 <= x && x < PlainField::MAP_WIDTH);
        DCHECK(0 <= y && y < PlainField::MAP_HEIGHT);
        return y * PlainField::MAP_WIDTH + x;
    }

    std::bitset<PlainField::MAP_HEIGHT * PlainField::MAP_WIDTH> field_;
};

#endif
