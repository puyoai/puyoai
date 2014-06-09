#ifndef CORE_FIELD_BIT_FIELD_H_
#define CORE_FIELD_BIT_FIELD_H_

#include <string.h>
#include "core/plain_field.h"

// FieldBitField is a bitset whose size if the same as field.
class FieldBitField {
public:
    FieldBitField() { memset(field_, 0, sizeof(field_)); }

    bool get(int x, int y) const { return field_[x] & (1 << y); }
    void set(int x, int y) { field_[x] |= (1 << y); }
    void clear(int x, int y) { field_[x] &= ~(1 << y); }

private:
    unsigned int field_[PlainField::MAP_WIDTH];
};

#endif
