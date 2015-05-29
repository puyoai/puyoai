#ifndef CORE_FIELD_CHECKER_H_
#define CORE_FIELD_CHECKER_H_

#include <cstdint>

#include <glog/logging.h>

#include "core/field_constant.h"

#ifdef EXPERIMENTAL_CORE_FIELD_USES_BIT_FIELD
#include "core/field_bits.h"
#define FieldChecker FieldBits
#else

// FieldChecker is a bitset whose size is the same as field.
class FieldChecker {
public:
    FieldChecker() : col_{} {}

    bool get(int x, int y) const
    {
        DCHECK(0 <= x && x < FieldConstant::MAP_WIDTH) << x;
        DCHECK(0 <= y && y < FieldConstant::MAP_HEIGHT) << y;
        return col_[x] & (1U << y);
    }

    void setBit(int x, int y, bool flag)
    {
        if (flag)
            set(x, y);
        else
            clear(x, y);
    }

    void set(int x, int y)
    {
        DCHECK(0 <= x && x < FieldConstant::MAP_WIDTH) << x;
        DCHECK(0 <= y && y < FieldConstant::MAP_HEIGHT) << y;
        col_[x] |= (1U << y);
    }

    void clear(int x, int y)
    {
        DCHECK(0 <= x && x < FieldConstant::MAP_WIDTH) << x;
        DCHECK(0 <= y && y < FieldConstant::MAP_HEIGHT) << y;
        col_[x] &= ~(1U << y);
    }

    bool operator()(int x, int y) const { return get(x, y); }

private:
    std::uint16_t col_[8];
};
#endif

#endif // CORE_FIELD_CHECKER_H_
