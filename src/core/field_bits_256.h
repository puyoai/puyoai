#ifndef CORE_FIELD_BITS_256_H_
#define CORE_FIELD_BITS_256_H_
#ifdef __AVX2__

#include <immintrin.h>

#include "core/field_bits.h"

class FieldBits256 {
public:
    enum class HighLow { LOW, HIGH };

    FieldBits256() : m_(_mm256_setzero_si256()) {}
    FieldBits256(FieldBits low, FieldBits high)
    {
        // TODO(mayah): We should have better algorithm here.
        m_ = _mm256_setzero_si256();
        m_ = _mm256_inserti128_si256(m_, low, 0);
        m_ = _mm256_inserti128_si256(m_, high, 1);
    }

    FieldBits256(HighLow highlow, int x, int y) : m_(onebit(highlow, x, y)) {}

    bool get(HighLow highlow, int x, int y) const { return !_mm256_testz_si256(onebit(highlow, x, y), m_); }

    FieldBits lowBits128() const { return _mm256_extracti128_si256(m_, 0); }
    FieldBits highBits128() const { return _mm256_extracti128_si256(m_, 1); }

private:
    static __m256i onebit(HighLow highlow, int x, int y);

    __m256i m_;
};

// static
inline __m256i FieldBits256::onebit(FieldBits256::HighLow highlow, int x, int y)
{
    DCHECK(0 <= x && x < 8 && 0 <= y && y < 16) << "x=" << x << " y=" << y;

    // TODO(mayah): Probably we have better solution.
    union {
        std::int16_t vs[16] {};
        __m256i m;
    };

    if (highlow == FieldBits256::HighLow::HIGH) {
        vs[x + 8] = 1 << y;
    } else {
        vs[x] = 1 << y;
    }

    return m;
}

#endif // __AVX2__
#endif // CORE_FIELD_BITS_256_H_
