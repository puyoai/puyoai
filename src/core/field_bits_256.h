#ifndef CORE_FIELD_BITS_256_H_
#define CORE_FIELD_BITS_256_H_
#ifdef __AVX2__

#include <string>
#include <utility>

#include <immintrin.h>

#include "base/avx.h"
#include "base/builtin.h"
#include "core/field_bits.h"

class FieldBits256 {
public:
    enum class HighLow { LOW, HIGH };

    FieldBits256() : m_(_mm256_setzero_si256()) {}
    FieldBits256(__m256i m) : m_(m) {}
    FieldBits256(FieldBits high, FieldBits low);
    FieldBits256(HighLow highlow, int x, int y) : m_(onebit(highlow, x, y)) {}

    operator __m256i&() { return m_; }
    __m256i& ymm() { return m_; }
    const __m256i& ymm() const { return m_; }

    bool get(HighLow highlow, int x, int y) const { return !_mm256_testz_si256(onebit(highlow, x, y), m_); }
    void set(HighLow highlow, int x, int y) { m_ = m_ | onebit(highlow, x, y); }
    void setHigh(int x, int y) { m_ = m_ | onebit(HighLow::HIGH, x, y); }
    void setLow(int x, int y) { m_ = m_ | onebit(HighLow::LOW, x, y); }

    void setAll(FieldBits256 m) { m_ = m_ | m; }

    std::pair<int, int> popcountHighLow() const;

    FieldBits low() const { return _mm256_castsi256_si128(m_); }
    FieldBits high() const { return _mm256_extracti128_si256(m_, 1); }

    FieldBits256 expand(FieldBits256 mask) const;
    FieldBits256 expand1(FieldBits256 mask) const;

    bool findVanishingBits(FieldBits256* bits) const;

    bool isEmpty() const { return _mm256_testz_si256(m_, m_); }
    std::string toString() const;

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

inline FieldBits256::FieldBits256(FieldBits high, FieldBits low)
{
    // See http://lists.cs.uiuc.edu/pipermail/cfe-commits/Week-of-Mon-20150518/129492.html
    // This works only in clang.
    // m_ = (__m256i) __builtin_shufflevector((__m128)low.xmm(), (__m128)high.xmm(), 0, 1, 2, 3, 4, 5, 6, 7);
    //
    m_ = _mm256_inserti128_si256(_mm256_castsi128_si256(low.xmm()), high.xmm(), 1);
}

inline FieldBits256 FieldBits256::expand(FieldBits256 mask) const
{
    FieldBits256 seed = m_;

    while (true) {
        FieldBits256 expanded = seed;
        expanded = _mm256_slli_epi16(seed, 1) | expanded;
        expanded = _mm256_srli_epi16(seed, 1) | expanded;
        expanded = _mm256_slli_si256(seed, 2) | expanded;
        expanded = _mm256_srli_si256(seed, 2) | expanded;
        expanded = mask & expanded;

        if (_mm256_testc_si256(seed, expanded))
            return expanded;
        seed = expanded;
    }

    // NOT_REACHED.
}

inline FieldBits256 FieldBits256::expand1(FieldBits256 mask) const
{
    FieldBits256 v1 = _mm256_slli_si256(m_, 2);
    FieldBits256 v2 = _mm256_srli_si256(m_, 2);
    FieldBits256 v3 = _mm256_slli_epi16(m_, 1);
    FieldBits256 v4 = _mm256_srli_epi16(m_, 1);
    return ((m_ | v1) | (v2 | v3) | v4) & mask;
}

inline
std::pair<int, int> FieldBits256::popcountHighLow() const
{
    avx::Decomposer256 d;
    d.m = m_;

    int low = popCount64(d.ui64[0]) + popCount64(d.ui64[1]);
    int high = popCount64(d.ui64[2]) + popCount64(d.ui64[3]);

    return std::make_pair(high, low);
}

inline bool FieldBits256::findVanishingBits(FieldBits256* vanishing) const
{
    DCHECK(vanishing) << "vanishing should not be nullptr";

    // See FieldBits::findVanishingSeed for the implementation details.

    __m256i u = _mm256_and_si256(_mm256_srli_epi16(m_, 1), m_);
    __m256i d = _mm256_and_si256(_mm256_slli_epi16(m_, 1), m_);
    __m256i l = _mm256_and_si256(_mm256_slli_si256(m_, 2), m_);
    __m256i r = _mm256_and_si256(_mm256_srli_si256(m_, 2), m_);

    __m256i ud_and = u & d;
    __m256i lr_and = l & r;
    __m256i ud_or = u | d;
    __m256i lr_or = l | r;

    __m256i twos = lr_and | ud_and | (ud_or & lr_or);
    __m256i two_d = _mm256_slli_epi16(twos, 1) & twos;
    __m256i two_l = _mm256_slli_si256(twos, 2) & twos;
    __m256i threes = (ud_and & lr_or) | (lr_and & ud_or);
    *vanishing = two_d | two_l | threes;

    if (vanishing->isEmpty())
        return false;

    __m256i two_u = _mm256_srli_epi16(twos, 1) & twos;
    __m256i two_r = _mm256_srli_si256(twos, 2) & twos;
    *vanishing = (*vanishing | two_u | two_r).expand1(m_);
    return true;
}

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
