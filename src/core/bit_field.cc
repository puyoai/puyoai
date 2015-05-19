#include "core/bit_field.h"

#include <smmintrin.h>

#include "core/score.h"

RensaResult BitField::simulate()
{
    int currentChain = 1;
    int score = 0;
    int frames = 0;
    int nthChainScore;
    bool quick = false;
    FieldBits erased;

    while ((nthChainScore = vanish(currentChain, &erased)) > 0) {
        currentChain += 1;
        score += nthChainScore;
        frames += FRAMES_VANISH_ANIMATION;
        int maxDrops = drop(erased);
        if (maxDrops > 0) {
            frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
        } else {
            quick = true;
        }
    }

    return RensaResult(currentChain - 1, score, frames, quick);
}

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
    FieldBits ojama(erased->expand1(bits(PuyoColor::OJAMA)));

    erased->setAll(ojama);
    m_[0].unsetAll(*erased);
    m_[1].unsetAll(*erased);
    m_[2].unsetAll(*erased);

    return 10 * numErased * calculateRensaBonusCoef(chainBonus(chain), longBonusCoef, colorBonus(numColors));
}

int BitField::drop(FieldBits erased)
{
    // bits   = b15 .. b9 b8 b7 .. b0

    // Consider y = 8.
    // v1 = and ( b15 .. b9 b8 b7 .. b0,
    //          (   0 ..  0  0   1 ..  1) = 0-0 b7-b0
    // v2 = and ( b15 .. b9 b8 b7 .. b0,
    //          (   1 ..  1  0  0 ..  0) = b15-b9 0-0
    // v3 = v2 >> 1 = 0 b15-b9 0-0
    // v4 = v1 | v3 = 0 b15-b9 b7-b0
    // new bits = BLEND(bits, v4, blender[y])

    FieldBits blender[16] {};
    erased.makeBlender(blender);

    for (int y = 12; y >= 1; --y) {
        if (blender[y].isEmpty())
            continue;
        const __m128i leftOnes = _mm_set1_epi16((1 << y) - 1);
        const __m128i rightOnes = _mm_set1_epi16(~((1 << (y + 1)) - 1));

        for (int i = 0; i < 3; ++i) {
            __m128i m = m_[i].xmm();
            __m128i v1 = _mm_and_si128(leftOnes, m);
            __m128i v2 = _mm_and_si128(rightOnes, m);
            __m128i v3 = _mm_srli_epi16(v2, 1);
            __m128i v4 = _mm_or_si128(v1, v3);
            // _mm_blend_epi16 takes const int for parameter, so let's use blendv_epi8.
            m = _mm_blendv_epi8(m, v4, blender[y].xmm());
            m_[i] = FieldBits(m);
        }
    }

    // TODO(mayah): Needs to return the max drop.
    return 0;
}
