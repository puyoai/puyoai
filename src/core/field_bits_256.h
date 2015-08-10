#ifndef CORE_FIELD_BITS_256_H_
#define CORE_FIELD_BITS_256_H_
#ifdef __AVX2__

#include <string>

#include <immintrin.h>

#include "core/field_bits.h"

class FieldBits256 {
public:
    enum class HighLow { LOW, HIGH };

    FieldBits256() : m_(_mm256_setzero_si256()) {}
    FieldBits256(__m256i m) : m_(m) {}
    FieldBits256(FieldBits high, FieldBits low)
    {
        // TODO(mayah): We should have better algorithm here.
        // See http://lists.cs.uiuc.edu/pipermail/cfe-commits/Week-of-Mon-20150518/129492.html
        m_ = _mm256_setzero_si256();
        m_ = _mm256_inserti128_si256(m_, low, 0);
        m_ = _mm256_inserti128_si256(m_, high, 1);
    }

    FieldBits256(HighLow highlow, int x, int y) : m_(onebit(highlow, x, y)) {}

    bool get(HighLow highlow, int x, int y) const { return !_mm256_testz_si256(onebit(highlow, x, y), m_); }

    FieldBits low() const { return _mm256_extracti128_si256(m_, 0); }
    FieldBits high() const { return _mm256_extracti128_si256(m_, 1); }

    bool isEmpty() const { return _mm256_testc_si256(_mm256_setzero_si256(), m_); }
    std::string toString() const; 

    __m256i& ymm() { return m_; }

    friend bool operator==(FieldBits256 lhs, FieldBits256 rhs) { return (lhs ^ rhs).isEmpty(); }
    friend bool operator!=(FieldBits256 lhs, FieldBits256 rhs) { return !(lhs == rhs); }

    friend FieldBits256 operator&(FieldBits256 lhs, FieldBits256 rhs) { return _mm256_and_si256(lhs.ymm(), rhs.ymm()); }
    friend FieldBits256 operator|(FieldBits256 lhs, FieldBits256 rhs) { return _mm256_or_si256(lhs.ymm(), rhs.ymm()); }
    friend FieldBits256 operator^(FieldBits256 lhs, FieldBits256 rhs) { return _mm256_xor_si256(lhs.ymm(), rhs.ymm()); }

    friend std::ostream& operator<<(std::ostream& os, const FieldBits256& bits) { return os << bits.toString(); }

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
