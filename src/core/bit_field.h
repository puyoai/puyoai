#ifndef CORE_BITS_FIELD_H_
#define CORE_BITS_FIELD_H_

#include <string>

#include "core/field_bits.h"
#include "core/puyo_color.h"
#include "core/rensa_result.h"

// BitsField is a field implementation that uses FieldBits.
class BitField {
public:
    BitField() {}
    explicit BitField(const PlainField&);
    explicit BitField(const std::string&);

    FieldBits bits(PuyoColor c) const;

    PuyoColor color(int x, int y) const;
    bool isColor(int x, int y, PuyoColor c) const { return bits(c).get(x, y); }
    bool isEmpty(int x, int y) const { return !(m_[0] | m_[1] | m_[2]).get(x, y); }
    void setColor(int x, int y, PuyoColor c);

    RensaResult simulate();

private:
    friend class BitFieldTest;

    // Vanishes puyos. Returns score. Erased puyos are put |erased|.
    int vanish(int nthChain, FieldBits* erased);
    // Drops puyos. Returns max drops.
    int drop(FieldBits erased);

    FieldBits m_[3];
};

inline
BitField::BitField(const PlainField& pf)
{
    __m128i m1 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(1)));
    __m128i m2 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(2)));
    __m128i m3 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(3)));
    __m128i m4 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(4)));
    __m128i m5 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(5)));
    __m128i m6 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(6)));

    for (int i = 0; i < NUM_PUYO_COLORS; ++i) {
        PuyoColor c = static_cast<PuyoColor>(i);
        if (c == PuyoColor::EMPTY || c == PuyoColor::WALL) {
            continue;
        }

        __m128i mask = _mm_set1_epi8(static_cast<char>(c));

        union {
            __m128i m;
            std::int16_t s[8];
        } xmm;

        xmm.s[0] = 0;
        xmm.s[1] = _mm_movemask_epi8(_mm_cmpeq_epi8(m1, mask));
        xmm.s[2] = _mm_movemask_epi8(_mm_cmpeq_epi8(m2, mask));
        xmm.s[3] = _mm_movemask_epi8(_mm_cmpeq_epi8(m3, mask));
        xmm.s[4] = _mm_movemask_epi8(_mm_cmpeq_epi8(m4, mask));
        xmm.s[5] = _mm_movemask_epi8(_mm_cmpeq_epi8(m5, mask));
        xmm.s[6] = _mm_movemask_epi8(_mm_cmpeq_epi8(m6, mask));
        xmm.s[7] = 0;

        if (ordinal(c) & 1)
            m_[0].setAll(FieldBits(xmm.m));
        if (ordinal(c) & 2)
            m_[1].setAll(FieldBits(xmm.m));
        if (ordinal(c) & 4)
            m_[2].setAll(FieldBits(xmm.m));
    }
}

inline
FieldBits BitField::bits(PuyoColor c) const
{
    switch (c) {
    case PuyoColor::EMPTY:  // = 0
        return FieldBits();
    case PuyoColor::OJAMA:  // = 1  001
        return FieldBits(_mm_andnot_si128(m_[2].xmm(), _mm_andnot_si128(m_[1].xmm(), m_[0].xmm())));
    case PuyoColor::WALL:
        return FieldBits();
    case PuyoColor::IRON:   // = 3  011
        return FieldBits(_mm_andnot_si128(m_[2].xmm(), _mm_and_si128(m_[1].xmm(), m_[0].xmm())));
    case PuyoColor::RED:    // = 4  100
        return FieldBits(_mm_andnot_si128(m_[0].xmm(), _mm_andnot_si128(m_[1].xmm(), m_[2].xmm())));
    case PuyoColor::BLUE:   // = 5  101
        return FieldBits(_mm_and_si128(m_[0].xmm(), _mm_andnot_si128(m_[1].xmm(), m_[2].xmm())));
    case PuyoColor::YELLOW: // = 6  110
        return FieldBits(_mm_andnot_si128(m_[0].xmm(), _mm_and_si128(m_[1].xmm(), m_[2].xmm())));
    case PuyoColor::GREEN:  // = 7  111
        return FieldBits(_mm_and_si128(m_[0].xmm(), _mm_and_si128(m_[1].xmm(), m_[2].xmm())));
    }

    CHECK(false);
    return FieldBits();
}

#endif // CORE_BIT_FIELD_H_
