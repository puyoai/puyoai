#ifndef CORE_FIELD_BITS_H_
#define CORE_FIELD_BITS_H_

#include <glog/logging.h>
#include <smmintrin.h>

#include "core/field_constant.h"
#include "core/plain_field.h"
#include "core/position.h"

// FieldBits is a bitset whose size is the same as field.
// Implemented using an xmm register.
class FieldBits {
public:
    static const __m128i FIELD_MASK;

    FieldBits() : m_(_mm_setzero_si128()) {}
    explicit FieldBits(__m128i m) : m_(m) {}
    FieldBits(int x, int y) : m_(onebit(x, y)) {}
    FieldBits(const PlainField&, PuyoColor);

    // These 3 methods are not so fast. Use only when necessary.
    bool get(int x, int y) const { return !_mm_testz_si128(onebit(x, y), m_); }
    void set(int x, int y) { m_ = _mm_or_si128(onebit(x, y), m_); }
    void unset(int x, int y) { m_ = _mm_andnot_si128(onebit(x, y), m_); }

    FieldBits masked() const { return FieldBits(_mm_and_si128(FIELD_MASK, m_)); }
    void setAll(const FieldBits& fb) { m_ = _mm_or_si128(fb.m_, m_); }

    bool isEmpty() const;

    int popcount() const;

    // Returns connected bits.
    FieldBits expand(FieldBits maskBits) const;
    // Returns connected bits. (more then 4 connected bits are not accurate.)
    FieldBits expand4(FieldBits maskBits) const;

    // Returns the seed bits for connected more that 4.
    // 1 connection might contain more than 1 seed.
    // e.g.
    // xxx...    .x....    .xx...
    // .xx... -> ...... or ...... or ...
    //
    // xxx... -> ...... (x isn't erased, so no seed bit.)
    //
    // When seed is expanded, It should be the same as all vanishing bits.
    FieldBits vanishingSeed() const;

    // Sets all the positions having 1 to |positions|.
    // |positions| should have 72 spaces at least. In some case, you need 128 spaces.
    // Returns length.
    int toPositions(Position positions[]) const;

    // Iterate all bits. Callback is void (FieldBits).
    template<typename Callback>
    void iterateBit(Callback) const;

    friend bool operator==(FieldBits lhs, FieldBits rhs)
    {
        return FieldBits(_mm_xor_si128(lhs.m_, rhs.m_)).isEmpty();
    }
    friend bool operator!=(FieldBits lhs, FieldBits rhs) { return !(lhs == rhs); }

private:
    static const __m128i s_table_[128];
    static __m128i onebit(int x, int y)
    {
        DCHECK(0 <= x && x < 8 && 0 <= y && y < 16) << "x=" << x << " y=" << y;
        return s_table_[x * 16 + y];
    }

    __m128i m_;
};

inline
FieldBits::FieldBits(const PlainField& pf, PuyoColor c)
{
    __m128i mask = _mm_set1_epi8(static_cast<char>(c));

    // TODO(mayah): should we use _mm_set_epi16? Which is faster?

    union {
        __m128i m;
        std::int16_t s[8];
    } xmm;

    xmm.s[0] = 0;
    for (int i = 1; i <= 6; ++i) {
        __m128i x = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(i)));
        xmm.s[i] = _mm_movemask_epi8(_mm_cmpeq_epi8(x, mask));
    }
    xmm.s[7] = 0;

    m_ = xmm.m;
}

inline
bool FieldBits::isEmpty() const
{
    __m128i allzero = _mm_setzero_si128();
    return _mm_testc_si128(allzero, m_);
}

inline
int FieldBits::popcount() const
{
    alignas(16) std::int64_t x[2];
    _mm_store_si128(reinterpret_cast<__m128i*>(x), m_);
    return __builtin_popcountll(x[0]) + __builtin_popcountll(x[1]);
}

inline
FieldBits FieldBits::expand(FieldBits maskBits) const
{
    const __m128i mask = maskBits.m_;
    __m128i seed = m_;

    while (true) {
        // TODO(mayah): We need to use another xmm register?
        // Register renaming will work well? I'm not sure...
        __m128i newSeed = seed;
        newSeed = _mm_or_si128(_mm_slli_epi16(seed, 1), newSeed);
        newSeed = _mm_or_si128(_mm_srli_epi16(seed, 1), newSeed);
        newSeed = _mm_or_si128(_mm_slli_si128(seed, 2), newSeed);
        newSeed = _mm_or_si128(_mm_srli_si128(seed, 2), newSeed);
        newSeed = _mm_and_si128(mask, newSeed);

        if (_mm_testc_si128(seed, newSeed))
            return FieldBits(seed);

        seed = newSeed;
    }

    // NOT_REACHED.
}

inline
FieldBits FieldBits::expand4(FieldBits maskBits) const
{
    const __m128i mask = maskBits.m_;
    __m128i m = m_;

    m = _mm_or_si128(_mm_and_si128(_mm_slli_si128(m, 2), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_srli_si128(m, 2), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_slli_epi16(m, 1), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_srli_epi16(m, 1), mask), m);

    m = _mm_or_si128(_mm_and_si128(_mm_slli_si128(m, 2), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_srli_si128(m, 2), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_slli_epi16(m, 1), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_srli_epi16(m, 1), mask), m);

    m = _mm_or_si128(_mm_and_si128(_mm_slli_si128(m, 2), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_srli_si128(m, 2), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_slli_epi16(m, 1), mask), m);
    m = _mm_or_si128(_mm_and_si128(_mm_srli_epi16(m, 1), mask), m);

    return FieldBits(m);
}

template<typename Callback>
inline void FieldBits::iterateBit(Callback callback) const
{
    const __m128i zero = _mm_setzero_si128();
    const __m128i downOnes = _mm_cvtsi64_si128(-1LL);
    const __m128i upOnes = _mm_slli_si128(downOnes, 8);

    __m128i current = m_;
    // upper is zero?
    while (!_mm_testz_si128(upOnes, current)) {
        // y = x & (-x)
        __m128i y = _mm_and_si128(current, _mm_sub_epi64(zero, current));
        __m128i z = _mm_and_si128(upOnes, y);
        callback(FieldBits(z));

        current = _mm_xor_si128(current, z);
    }

    while (!_mm_testz_si128(downOnes, current)) {
        // y = x & (-x)
        __m128i y = _mm_and_si128(current, _mm_sub_epi64(zero, current));
        __m128i z = _mm_and_si128(downOnes, y);
        callback(FieldBits(z));

        current = _mm_xor_si128(current, z);
    }
}

inline
int FieldBits::toPositions(Position ps[]) const
{
    int pos = 0;
    alignas(16) std::int64_t vs[2];
    _mm_store_si128(reinterpret_cast<__m128i*>(vs), m_);

    while (vs[0]) {
        int bit = __builtin_ctzll(vs[0]);
        int x = bit >> 4;
        int y = bit & 0xF;
        ps[pos++] = Position(x, y);
        vs[0] = vs[0] & (vs[0] - 1);
    }

    while (vs[1]) {
        int bit = __builtin_ctzll(vs[1]);
        int x = 4 + (bit >> 4);
        int y = bit & 0xF;
        ps[pos++] = Position(x, y);
        vs[1] = vs[1] & (vs[1] - 1);
    }

    return pos;
}

#endif // CORE_FIELD_BITS_H_
