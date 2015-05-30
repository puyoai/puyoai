#ifndef CORE_FIELD_BITS_H_
#define CORE_FIELD_BITS_H_

#include <glog/logging.h>
#include <smmintrin.h>
#include <string>

#include "core/field_constant.h"
#include "core/position.h"
#include "core/puyo_color.h"

class PlainField;

// FieldBits is a bitset whose size is the same as field.
// Implemented using an xmm register.
class FieldBits {
public:
    FieldBits() : m_(_mm_setzero_si128()) {}
    FieldBits(__m128i m) : m_(m) {}
    FieldBits(int x, int y) : m_(onebit(x, y)) {}
    FieldBits(const PlainField&, PuyoColor);

    operator __m128i&() { return m_; }
    __m128i& xmm() { return m_; }
    const __m128i& xmm() const { return m_; }

    // These 3 methods are not so fast. Use only when necessary.
    bool get(int x, int y) const { return !_mm_testz_si128(onebit(x, y), m_); }
    void set(int x, int y) { m_ = _mm_or_si128(onebit(x, y), m_); }
    void unset(int x, int y) { m_ = _mm_andnot_si128(onebit(x, y), m_); }

    void setAll(const FieldBits& fb) { m_ = _mm_or_si128(fb.m_, m_); }
    void unsetAll(const FieldBits& fb) { m_ = _mm_andnot_si128(fb.m_, m_); }

    bool isEmpty() const;
    bool testz(FieldBits bits) const { return _mm_testz_si128(m_, bits.m_); }

    int popcount() const;
    // Returns the bit-wise or of 8x16bits.
    int horizontalOr16() const;

    // Returns the masked FieldBits where the region of visible field is taken.
    FieldBits maskedField12() const;
    // Returns the masked FieldBits where the region of visible field + 13th row is taken.
    FieldBits maskedField13() const;

    // Returns m_ & mask.
    FieldBits mask(FieldBits mask) const { return m_ & mask; }
    // Returns m_ & ~mask
    FieldBits notmask(FieldBits mask) const { return _mm_andnot_si128(mask, m_); }

    // Returns all connected bits.
    FieldBits expand(FieldBits mask) const;
    // Returns connected bits. (more then 4 connected bits are not accurate.)
    FieldBits expand4(FieldBits mask) const;

    // Returns bits where edge is expanded.
    // This might contain the original bits, so you'd like to take mask.
    // e.g.
    // ......      ..xx..    ..xx..
    // ..xx..  --> .x..x. or .xxxx.
    // ......      ..xx..    ..xx..
    FieldBits expandEdge() const;

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

    // Iterate all bits. Callback is FieldBits (FieldBits).
    // The FieldBits the callback returned will be excluded from the iteration.
    template<typename Callback>
    void iterateBitWithMasking(Callback) const;

    std::string toString() const;

    friend bool operator==(FieldBits lhs, FieldBits rhs) { return (lhs ^ rhs).isEmpty(); }
    friend bool operator!=(FieldBits lhs, FieldBits rhs) { return !(lhs == rhs); }

    friend FieldBits operator&(FieldBits lhs, FieldBits rhs) { return _mm_and_si128(lhs, rhs); }
    friend FieldBits operator|(FieldBits lhs, FieldBits rhs) { return _mm_or_si128(lhs, rhs); }
    friend FieldBits operator^(FieldBits lhs, FieldBits rhs) { return _mm_xor_si128(lhs, rhs); }

private:
    static __m128i onebit(int x, int y);

    __m128i m_;
};

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
int FieldBits::horizontalOr16() const
{
    __m128i x = _mm_or_si128(_mm_srli_si128(m_, 8), m_);
    x = _mm_or_si128(_mm_srli_si128(x, 4), x);
    x = _mm_or_si128(_mm_srli_si128(x, 2), x);
    return _mm_cvtsi128_si32(x) & 0xFFFF;
}

inline
FieldBits FieldBits::maskedField12() const
{
    const auto mask = _mm_set_epi16(0, 0x1FFE, 0x1FFE, 0x1FFE, 0x1FFE, 0x1FFE, 0x1FFE, 0);
    return FieldBits(_mm_and_si128(mask, m_));
}

inline
FieldBits FieldBits::maskedField13() const
{
    const auto mask = _mm_set_epi16(0, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0);
    return FieldBits(_mm_and_si128(mask, m_));
}

inline
FieldBits FieldBits::expand(FieldBits mask) const
{
    __m128i seed = m_;

    while (true) {
        __m128i expanded = _mm_or_si128(_mm_slli_epi16(seed, 1), seed);
        expanded = _mm_or_si128(_mm_srli_epi16(seed, 1), expanded);
        expanded = _mm_or_si128(_mm_slli_si128(seed, 2), expanded);
        expanded = _mm_or_si128(_mm_srli_si128(seed, 2), expanded);
        expanded = _mm_and_si128(mask, expanded);

        if (_mm_testc_si128(seed, expanded))
            return expanded;
        seed = expanded;
    }

    // NOT_REACHED.
}

inline
FieldBits FieldBits::expand4(FieldBits mask) const
{
    FieldBits m = m_;

    FieldBits v1 = _mm_slli_si128(m, 2);
    FieldBits v2 = _mm_srli_si128(m, 2);
    FieldBits v3 = _mm_slli_epi16(m, 1);
    FieldBits v4 = _mm_srli_epi16(m, 1);
    m = (((v1 | v2) | (v3 | v4)) & mask) | m;

    v1 = _mm_slli_si128(m, 2);
    v2 = _mm_srli_si128(m, 2);
    v3 = _mm_slli_epi16(m, 1);
    v4 = _mm_srli_epi16(m, 1);
    m = (((v1 | v2) | (v3 | v4)) & mask) | m;

    v1 = _mm_slli_si128(m, 2);
    v2 = _mm_srli_si128(m, 2);
    v3 = _mm_slli_epi16(m, 1);
    v4 = _mm_srli_epi16(m, 1);
    m = (((v1 | v2) | (v3 | v4)) & mask) | m;

    return m;
}

inline
FieldBits FieldBits::expandEdge() const
{
    __m128i m1 = _mm_slli_epi16(m_, 1);
    __m128i m2 = _mm_srli_epi16(m_, 1);
    __m128i m3 = _mm_slli_si128(m_, 2);
    __m128i m4 = _mm_srli_si128(m_, 2);

    return _mm_or_si128(_mm_or_si128(m1, m2), _mm_or_si128(m3, m4));
}

inline
FieldBits FieldBits::vanishingSeed() const
{
    //  x
    // xox              -- o is 3-connected
    //
    // xoox  ox   x oo
    //      xo  xoo oo  -- o is 2-connected.
    //
    // So, one 3-connected piece or two 2-connected pieces are necessary and sufficient.

    __m128i u = _mm_and_si128(_mm_slli_epi16(m_, 1), m_);
    __m128i d = _mm_and_si128(_mm_srli_epi16(m_, 1), m_);
    __m128i l = _mm_and_si128(_mm_slli_si128(m_, 2), m_);
    __m128i r = _mm_and_si128(_mm_srli_si128(m_, 2), m_);

    __m128i ud_and = _mm_and_si128(u, d);
    __m128i lr_and = _mm_and_si128(l, r);
    __m128i ud_or = _mm_or_si128(u, d);
    __m128i lr_or = _mm_or_si128(l, r);

    __m128i threes = _mm_or_si128(
        _mm_and_si128(ud_and, lr_or),
        _mm_and_si128(lr_and, ud_or));

    __m128i twos = _mm_or_si128(_mm_or_si128(
        _mm_and_si128(ud_or, lr_or), ud_and), lr_and);

    __m128i two_u = _mm_and_si128(_mm_slli_epi16(twos, 1), twos);
    __m128i two_l = _mm_and_si128(_mm_slli_si128(twos, 2), twos);
    __m128i two_twos = _mm_or_si128(two_u, two_l);

    return FieldBits(_mm_or_si128(threes, two_twos));
}

template<typename Callback>
inline void FieldBits::iterateBitWithMasking(Callback callback) const
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
        FieldBits mask = callback(FieldBits(z));
        current = _mm_andnot_si128(mask.xmm(), current);
    }

    while (!_mm_testz_si128(downOnes, current)) {
        // y = x & (-x)
        __m128i y = _mm_and_si128(current, _mm_sub_epi64(zero, current));
        __m128i z = _mm_and_si128(downOnes, y);
        FieldBits mask = callback(FieldBits(z));
        current = _mm_andnot_si128(mask.xmm(), current);
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

// static
inline
__m128i FieldBits::onebit(int x, int y)
{
    DCHECK(0 <= x && x < 8 && 0 <= y && y < 16) << "x=" << x << " y=" << y;

#if 1
    int shift = ((x << 4) | y) & 0x3F;
    std::uint64_t hi = x >> 2;
    std::uint64_t lo = hi ^ 1;
    return _mm_set_epi64x(hi << shift, lo << shift);
#else
    // NOTE: the 3rd argument of _mm_insert_epi16 should be an immediate.
    // Mac clang and Linux gcc accepts non immediate value, however, it generates table jump code.
    // cygwin environment does not accept non immediate value.
    // So, we adopt switch case sentene here.
    switch (x) {
    case 0: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 0);
    case 1: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 1);
    case 2: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 2);
    case 3: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 3);
    case 4: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 4);
    case 5: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 5);
    case 6: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 6);
    case 7: return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 7);
    default:
        DCHECK(false);
        return _mm_insert_epi16(_mm_setzero_si128(), 1 << y, 0);
    }
#endif
}

#endif // CORE_FIELD_BITS_H_
