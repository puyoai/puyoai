#ifndef CORE_FIELD_BITS_H_
#define CORE_FIELD_BITS_H_

#include <glog/logging.h>
#include <smmintrin.h>

#include "core/field_constant.h"

// FieldBits is a bitset whose size is the same as field.
// Implemented using an xmm register.
class FieldBits {
public:
    FieldBits() : m_(_mm_setzero_si128()) {}
    explicit FieldBits(__m128i m) : m_(m) {}

    __m128i& xmm() { return m_; }

    // These 3 methods are not so fast. Use only when necessary.
    bool get(int x, int y) const { return !_mm_testz_si128(onebit(x, y), m_); }
    void set(int x, int y) { m_ = _mm_or_si128(onebit(x, y), m_); }
    void unset(int x, int y) { m_ = _mm_andnot_si128(onebit(x, y), m_); }

    // Masks visible (12x6) field.
    FieldBits masked() const { return FieldBits(_mm_and_si128(s_field_mask_, m_)); }

    int popcount() const;

    // Returns connected bits from (x, y).
    FieldBits expand(int x, int y) const;

private:
    static const __m128i s_field_mask_;
    static const __m128i s_table_[128];
    static __m128i onebit(int x, int y)
    {
        DCHECK(0 <= x && x < 8 && 0 <= y && y < 16) << "x=" << x << " y=" << y;
        return s_table_[x * 16 + y];
    }

    __m128i m_;
};

inline
int FieldBits::popcount() const
{
    alignas(16) std::int64_t x[2];
    _mm_store_si128(reinterpret_cast<__m128i*>(x), m_);
    return __builtin_popcountll(x[0]) + __builtin_popcountll(x[1]);
}

inline
FieldBits FieldBits::expand(int x, int y) const
{
    __m128i connected = onebit(x, y);

    while (true) {
        // TODO(mayah): We need to use another xmm register?
        // Register renaming will work well? I'm not sure...
        __m128i newConnected = connected;
        newConnected = _mm_or_si128(_mm_slli_epi16(connected, 1), newConnected);
        newConnected = _mm_or_si128(_mm_srli_epi16(connected, 1), newConnected);
        newConnected = _mm_or_si128(_mm_slli_si128(connected, 2), newConnected);
        newConnected = _mm_or_si128(_mm_srli_si128(connected, 2), newConnected);
        newConnected = _mm_and_si128(m_, newConnected);

        if (_mm_testc_si128(connected, newConnected))
            return FieldBits(newConnected);

        connected = newConnected;
    }

    return FieldBits(connected);
}

#endif // CORE_FIELD_BITS_H_
