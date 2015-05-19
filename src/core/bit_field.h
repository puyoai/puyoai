#ifndef CORE_BITS_FIELD_H_
#define CORE_BITS_FIELD_H_

#include "core/field_bits.h"
#include "core/puyo_color.h"
#include "core/rensa_result.h"

// BitsField is a field implementation that uses FieldBits.
// TODO(mayah): Implement this.
class BitField {
public:
    explicit BitField(const PlainField&);

    const FieldBits& bits(PuyoColor c) const;

    bool isColor(int x, int y, PuyoColor c) const { return bits(c).get(x, y); }

    RensaResult simulate();

private:
    friend class BitFieldTest;

    // Vanishes puyos. Returns score. Erased puyos are put |erased|.
    int vanish(int nthChain, FieldBits* erased);
    // Drops puyos. Returns max drops.
    int drop(FieldBits erased);

    static const FieldBits s_empty_;
    FieldBits colors_[NUM_PUYO_COLORS];
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
            colors_[ordinal(c)] = FieldBits();
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

        colors_[ordinal(c)] = FieldBits(xmm.m);
    }
}

inline
const FieldBits& BitField::bits(PuyoColor c) const
{
    DCHECK(c != PuyoColor::WALL && c != PuyoColor::EMPTY);
    return colors_[ordinal(c)];
}

#endif // CORE_BIT_FIELD_H_
