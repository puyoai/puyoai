#ifndef CORE_FIELD_BITS_H_
#define CORE_FIELD_BITS_H_

#include <cstdint>

#include <glog/logging.h>
#include <smmintrin.h>

#include "core/field_constant.h"

// FieldBits is a bitset whose size is the same as field.
class FieldBits {
public:
    FieldBits() : m_(_mm_setzero_si128()) {}

    __m128i& xmm() { return m_; }

    bool get(int x, int y) const { return !_mm_testz_si128(s_table_[tableIndex(x, y)], m_); }
    void set(int x, int y) { m_ = _mm_or_si128(s_table_[tableIndex(x, y)], m_); }
    void unset(int x, int y) { m_ = _mm_andnot_si128(s_table_[tableIndex(x, y)], m_); }

private:
    static __m128i s_table_[128];

    static int tableIndex(int x, int y)
    {
        DCHECK(0 <= x && x < 8 && 0 <= y && y < 16) << "x=" << x << " y=" << y;
        return x * 16 + y;
    }

    __m128i m_;
};

#endif // CORE_FIELD_BITS_H_
