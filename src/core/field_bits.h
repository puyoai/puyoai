#ifndef CORE_FIELD_BITS_H_
#define CORE_FIELD_BITS_H_

#include <smmintrin.h>

#include <ostream>
#include <string>

#include <glog/logging.h>

#include "base/builtin.h"
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
    // The position that has |c| character will become '1'.
    explicit FieldBits(const std::string&, char c = '1');

    operator const __m128i&() const { return m_; }
    operator __m128i&() { return m_; }
    __m128i& xmm() { return m_; }
    const __m128i& xmm() const { return m_; }

    // These 4 methods are not so fast. Use only when necessary.
    bool get(int x, int y) const { return !_mm_testz_si128(onebit(x, y), m_); }
    void set(int x, int y) { m_ = _mm_or_si128(onebit(x, y), m_); }
    void unset(int x, int y) { m_ = _mm_andnot_si128(onebit(x, y), m_); }
    void clear(int x, int y) { unset(x, y); }

    void setBit(int x, int y, bool b) { if (b) set(x, y); else unset(x, y); }

    void setAll(const FieldBits& fb) { m_ = _mm_or_si128(fb.m_, m_); }
    void unsetAll(const FieldBits& fb) { m_ = _mm_andnot_si128(fb.m_, m_); }

    bool isEmpty() const { return _mm_testz_si128(m_, m_); }
    bool testz(FieldBits bits) const { return _mm_testz_si128(m_, bits.m_); }

    // Returns the number of 1 bits in this FieldBits.
    int popcount() const;
    // Returns the bit-wise or of 8x16bits.
    int horizontalOr16() const;
    // Returns the Y of heightest bit.
    // If isEmpty(), -1 will be returned.
    int highestHeight() const;

    void countConnection(int* count2, int* count3) const;

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
    FieldBits expand1(FieldBits mask) const;
    // Returns connected bits. (more then 4 connected bits are not accurate.)
    FieldBits expand4(FieldBits mask) const;

    // Returns bits where edge is expanded.
    // This might contain the original bits, so you'd like to take mask.
    // e.g.
    // ......      ..xx..    ..xx..
    // ..xx..  --> .x..x. or .xxxx.
    // ......      ..xx..    ..xx..
    FieldBits expandEdge() const;

    // Returns true if there are 4-connected bits.
    // Such bits are copied to |vanishing|.
    // |vanishing| must not be nullptr.
    bool findVanishingBits(FieldBits* vanishing) const;

    bool hasVanishingBits() const;

    FieldBits mirror() const;

    // Sets all the positions having 1 to |positions|.
    // |positions| should have 72 spaces at least. In some case, you need 128 spaces.
    // Returns length.
    int toPositions(Position positions[]) const;

    // Iterate all bits. Callback is FieldBits (FieldBits).
    // The FieldBits the callback returned will be excluded from the iteration.
    template<typename Callback>
    void iterateBitWithMasking(Callback) const;

    // Iterate all bit positions. Callback is void (int x, int y).
    template<typename Callback>
    void iterateBitPositions(Callback) const;

    size_t hash() const;
    std::string toString() const;

    friend bool operator==(FieldBits lhs, FieldBits rhs) { return (lhs ^ rhs).isEmpty(); }
    friend bool operator!=(FieldBits lhs, FieldBits rhs) { return !(lhs == rhs); }

    friend std::ostream& operator<<(std::ostream& os, const FieldBits& bits) { return os << bits.toString(); }

    friend FieldBits operator&(FieldBits lhs, FieldBits rhs) { return _mm_and_si128(lhs, rhs); }
    friend FieldBits operator|(FieldBits lhs, FieldBits rhs) { return _mm_or_si128(lhs, rhs); }
    friend FieldBits operator^(FieldBits lhs, FieldBits rhs) { return _mm_xor_si128(lhs, rhs); }

    const static FieldBits FIELD_MASK_13;
    const static FieldBits FIELD_MASK_12;

private:
    static __m128i onebit(int x, int y);

    __m128i m_;
};

namespace std {

template<>
struct hash<FieldBits>
{
    size_t operator()(const FieldBits& bits) const { return bits.hash(); }
};

}

inline size_t FieldBits::hash() const
{
    union {
        __m128i m;
        uint64_t xs[2];
    };
    m = xmm();

    return xs[0] + (xs[1] * 100000009);
}

inline
int FieldBits::popcount() const
{
    alignas(16) std::int64_t x[2];
    _mm_store_si128(reinterpret_cast<__m128i*>(x), m_);
    return popCount64(x[0]) + popCount64(x[1]);
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
int FieldBits::highestHeight() const
{
    int or16 = horizontalOr16();
    if (or16 == 0)
        return -1;

    return 31 - countLeadingZeros32(or16);
}

inline
FieldBits FieldBits::maskedField12() const
{
    return FIELD_MASK_12 & m_;
}

inline
FieldBits FieldBits::maskedField13() const
{
    return FIELD_MASK_13 & m_;
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

inline FieldBits FieldBits::expand1(FieldBits mask) const
{
    FieldBits v1 = _mm_slli_si128(m_, 2);
    FieldBits v2 = _mm_srli_si128(m_, 2);
    FieldBits v3 = _mm_slli_epi16(m_, 1);
    FieldBits v4 = _mm_srli_epi16(m_, 1);

    // NOTE: clang++ (3.4) emits an assembly that looks
    // parallelly executable. But g++ (4.8.4) does not.
    // Anyway, out-of-order execution will hide this difference.
    return ((m_ | v1) | (v2 | v3) | v4) & mask;
}

inline
FieldBits FieldBits::expand4(FieldBits mask) const
{
    FieldBits m = m_;

    // 3 times, not 4 times.
    m = m.expand1(mask);
    m = m.expand1(mask);
    m = m.expand1(mask);

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
bool FieldBits::findVanishingBits(FieldBits* vanishing) const
{
    //  x
    // xox              -- o is 3-connected
    //
    // xoox  ox   x oo
    //      xo  xoo oo  -- o is 2-connected.
    //
    // So, one 3-connected piece or two 2-connected pieces are necessary and sufficient.
    //
    // Also, 1-connected won't be connected to each other in vanishing case.
    // So, after this, expand1() should be enough.

    DCHECK(vanishing) << "bits must not be nullptr";

    FieldBits u = _mm_and_si128(_mm_srli_epi16(m_, 1), m_);
    FieldBits d = _mm_and_si128(_mm_slli_epi16(m_, 1), m_);
    FieldBits l = _mm_and_si128(_mm_slli_si128(m_, 2), m_);
    FieldBits r = _mm_and_si128(_mm_srli_si128(m_, 2), m_);

    FieldBits ud_and = u & d;
    FieldBits lr_and = l & r;
    FieldBits ud_or = u | d;
    FieldBits lr_or = l | r;

    FieldBits threes = (ud_and & lr_or) | (lr_and & ud_or);
    FieldBits twos = ud_and | lr_and | (ud_or & lr_or);

    FieldBits two_d = _mm_slli_epi16(twos, 1) & twos;
    FieldBits two_l = _mm_slli_si128(twos, 2) & twos;

    *vanishing = threes | two_d | two_l;

    if (vanishing->isEmpty())
        return false;

    FieldBits two_u = _mm_srli_epi16(twos, 1) & twos;
    FieldBits two_r = _mm_srli_si128(twos, 2) & twos;
    *vanishing = (*vanishing | two_u | two_r).expand1(m_);
    return true;
}

inline
bool FieldBits::hasVanishingBits() const
{
    FieldBits u = _mm_and_si128(_mm_srli_epi16(m_, 1), m_);
    FieldBits d = _mm_and_si128(_mm_slli_epi16(m_, 1), m_);
    FieldBits l = _mm_and_si128(_mm_slli_si128(m_, 2), m_);
    FieldBits r = _mm_and_si128(_mm_srli_si128(m_, 2), m_);

    FieldBits ud_and = u & d;
    FieldBits lr_and = l & r;
    FieldBits ud_or = u | d;
    FieldBits lr_or = l | r;

    FieldBits threes = (ud_and & lr_or) | (lr_and & ud_or);
    FieldBits twos = ud_and | lr_and | (ud_or & lr_or);

    FieldBits two_d = _mm_slli_epi16(twos, 1) & twos;
    FieldBits two_l = _mm_slli_si128(twos, 2) & twos;

    FieldBits vanishing = threes | two_d | two_l;

    return !vanishing.isEmpty();
}

inline
void FieldBits::countConnection(int* count2, int* count3) const
{
    FieldBits mask = maskedField12();

    FieldBits u = _mm_and_si128(_mm_srli_epi16(mask, 1), mask);
    FieldBits d = _mm_and_si128(_mm_slli_epi16(mask, 1), mask);
    FieldBits l = _mm_and_si128(_mm_slli_si128(mask, 2), mask);
    FieldBits r = _mm_and_si128(_mm_srli_si128(mask, 2), mask);

    FieldBits ud_and = u & d;
    FieldBits lr_and = l & r;
    FieldBits ud_or = u | d;
    FieldBits lr_or = l | r;

    FieldBits three = (ud_or & lr_or) | ud_and | lr_and;
    FieldBits two = (u | l).notmask(three.expand(mask));

    *count2 = two.popcount();
    *count3 = three.popcount();
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

template<typename Callback>
void FieldBits::iterateBitPositions(Callback callback) const
{
    alignas(16) std::int64_t vs[2];
    _mm_store_si128(reinterpret_cast<__m128i*>(vs), m_);

    while (vs[0]) {
        int bit = countTrailingZeros64(vs[0]);
        int x = bit >> 4;
        int y = bit & 0xF;
        callback(x, y);
        vs[0] = vs[0] & (vs[0] - 1);
    }

    while (vs[1]) {
        int bit = countTrailingZeros64(vs[1]);
        int x = 4 + (bit >> 4);
        int y = bit & 0xF;
        callback(x, y);
        vs[1] = vs[1] & (vs[1] - 1);
    }
}

inline
int FieldBits::toPositions(Position ps[]) const
{
    int pos = 0;
    iterateBitPositions([&](int x, int y) {
        ps[pos++] = Position(x, y);
    });
    return pos;
}

// static
inline
__m128i FieldBits::onebit(int x, int y)
{
    DCHECK(0 <= x && x < 8 && 0 <= y && y < 16) << "x=" << x << " y=" << y;

    int shift = ((x << 4) | y) & 0x3F;
    std::uint64_t hi = x >> 2;
    std::uint64_t lo = hi ^ 1;
    return _mm_set_epi64x(hi << shift, lo << shift);
}

#endif // CORE_FIELD_BITS_H_
