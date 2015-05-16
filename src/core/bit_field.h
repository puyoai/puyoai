#ifndef CORE_BITS_FIELD_H_
#define CORE_BITS_FIELD_H_

#include "core/field_bits.h"
#include "core/puyo_color.h"

// BitsField is a field implementation that uses FieldBits.
// TODO(mayah): Implement this.
class BitField {
public:
    explicit BitField(const PlainField&);

    const FieldBits& bits(PuyoColor c) const;

    bool isColor(int x, int y, PuyoColor c) const { return bits(c).get(x, y); }

private:
    static const FieldBits s_empty_;

    FieldBits colors_[4];
    FieldBits ojama_;
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

    for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
        PuyoColor c = NORMAL_PUYO_COLORS[i];
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

        colors_[i] = FieldBits(_mm_and_si128(FieldBits::FIELD_MASK, xmm.m));
    }

    {
        __m128i mask = _mm_set1_epi8(static_cast<char>(PuyoColor::OJAMA));

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

        ojama_ = FieldBits(_mm_and_si128(FieldBits::FIELD_MASK, xmm.m));
    }
}

inline
const FieldBits& BitField::bits(PuyoColor c) const
{
    switch (c) {
        case PuyoColor::RED:
            return colors_[0];
        case PuyoColor::BLUE:
            return colors_[1];
        case PuyoColor::YELLOW:
            return colors_[2];
        case PuyoColor::GREEN:
            return colors_[3];
        case PuyoColor::OJAMA:
            return ojama_;
        default:
            return s_empty_;
    }
}

#endif // CORE_BIT_FIELD_H_
