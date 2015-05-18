#include "core/bit_field.h"

#include <smmintrin.h>

#include "core/score.h"

// static
const FieldBits BitField::s_empty_;

int BitField::vanish(int chain, FieldBits* erased)
{
    int numErased = 0;
    int numColors = 0;
    int longBonusCoef = 0;

    *erased = FieldBits();

    for (PuyoColor c : NORMAL_PUYO_COLORS) {
        FieldBits mask = bits(c).masked();
        FieldBits seed = mask.vanishingSeed();

        if (seed.isEmpty())
            continue;

        ++numColors;
        seed.iterateBit([&](FieldBits x) {
            if (mask.testz(x))
                return;

            FieldBits expanded = x.expand4(mask).expand(mask);
            int count = expanded.popcount();
            numErased += count;
            longBonusCoef += longBonus(count);
            erased->setAll(expanded);
            mask.unsetAll(expanded);
        });
    }

    // Removes ojama.
    __m128i expanded = erased->xmm();
    expanded = _mm_or_si128(_mm_slli_epi16(erased->xmm(), 1), expanded);
    expanded = _mm_or_si128(_mm_srli_epi16(erased->xmm(), 1), expanded);
    expanded = _mm_or_si128(_mm_slli_si128(erased->xmm(), 2), expanded);
    expanded = _mm_or_si128(_mm_srli_si128(erased->xmm(), 2), expanded);
    FieldBits ojama(_mm_and_si128(bits(PuyoColor::OJAMA).xmm(), expanded));

    erased->setAll(ojama);
    for (auto& c : colors_)
        c.unsetAll(*erased);

    return 10 * numErased * calculateRensaBonusCoef(chainBonus(chain) , longBonusCoef, colorBonus(numColors));
}
