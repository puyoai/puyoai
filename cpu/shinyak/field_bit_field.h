#ifndef __FIELD_BIT_FIELD_H_
#define __FIELD_BIT_FIELD_H_

#include <string.h>
#include "field.h"

class FieldBitField
{
public:
    FieldBitField() {
        memset(m_field, 0, sizeof(m_field));
    }

    bool get(int x, int y) const {
        return m_field[x] & (1 << y);
    }

    void set(int x, int y) {
        m_field[x] |= (1 << y);
    }

    void clear(int x, int y) {
        m_field[x] &= ~(1 << y);
    }

private:
    unsigned int m_field[Field::MAP_WIDTH];
};

#endif
