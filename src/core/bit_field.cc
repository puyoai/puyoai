#include "core/bit_field.h"

#include <smmintrin.h>
#include <glog/logging.h>

#include "core/frame.h"
#include "core/score.h"

using namespace std;

BitField::BitField(const string& str)
{
    int counter = 0;
    for (int i = str.length() - 1; i >= 0; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = toPuyoColor(str[i]);
        setColor(x, y, c);
        counter++;
    }
}

void BitField::setColor(int x, int y, PuyoColor c)
{
    int cc = static_cast<int>(c);
    for (int i = 0; i < 3; ++i) {
        if (cc & (1 << i))
            m_[i].set(x, y);
        else
            m_[i].unset(x, y);

    }
}

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

        // fast path. In most cases, 8>= puyos won't be erased.
        // When 7<= puyos are erased, it won't be separated.
        {
            FieldBits expanded = seed.expand(mask);
            int popcount = expanded.popcount();
            if (popcount <= 7) {
                numErased += popcount;
                longBonusCoef += longBonus(popcount);
                erased->setAll(expanded);
                mask.unsetAll(expanded);
                continue;
            }
        }

        // slow path...
        seed.iterateBitWithMasking([&](FieldBits x) -> FieldBits {
            if (mask.testz(x))
                return x;

            FieldBits expanded = x.expand(mask);
            int count = expanded.popcount();
            numErased += count;
            longBonusCoef += longBonus(count);
            erased->setAll(expanded);
            mask.unsetAll(expanded);
            return expanded;
        });
    }

    if (numColors == 0)
        return 0;

    // Removes ojama.
    FieldBits ojama(erased->expand1ForOjama(bits(PuyoColor::OJAMA)));

    erased->setAll(ojama);
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
    // new bits = BLEND(bits, v4, blender)
    const __m128i zero = _mm_setzero_si128();
    const __m128i ones = _mm_cmpeq_epi8(zero, zero);
    const __m128i whole = _mm_andnot_si128(erased.xmm(),
        _mm_or_si128(m_[0].xmm(), _mm_or_si128(m_[1].xmm(), m_[2].xmm())));

    int wholeErased = erased.horizontalOr16();
    int maxY = 31 - __builtin_clz(wholeErased);
    int minY = __builtin_ctz(wholeErased);

    DCHECK(1 <= minY && minY <= maxY && maxY <= 12)
        << "minY=" << minY << ' ' << "maxY=" << maxY << endl << erased.toString();

    //        MSB      y      LSB
    // line  : 0 ... 0 1 0 ... 0
    // right : 0 ... 0 0 1 ... 1
    // left  : 1 ... 1 0 0 ... 0

    __m128i line = _mm_set1_epi16(1 << (maxY + 1));
    __m128i rightOnes = _mm_set1_epi16((1 << (maxY + 1)) - 1);
    __m128i leftOnes = _mm_set1_epi16(~((1 << ((maxY + 1) + 1)) - 1));

    __m128i dropAmount = zero;
    // For each line, -1 if there exists dropping puyo, 0 otherwise.
    __m128i exists = _mm_cmpeq_epi16(_mm_and_si128(line, whole), line);

    for (int y = maxY; y >= minY; --y) {
        line = _mm_srli_epi16(line, 1);
        rightOnes = _mm_srai_epi16(rightOnes, 1);
        leftOnes = _mm_srai_epi16(leftOnes, 1);   // needs arithmetic shift.

        // for each line, -1 if drop, 0 otherwise.
        __m128i blender = _mm_xor_si128(_mm_cmpeq_epi16(_mm_and_si128(line, erased.xmm()), zero), ones);
        DCHECK(!FieldBits(blender).isEmpty());

        for (int i = 0; i < 3; ++i) {
            __m128i m = m_[i].xmm();
            __m128i v1 = _mm_and_si128(rightOnes, m);
            __m128i v2 = _mm_and_si128(leftOnes, m);
            __m128i v3 = _mm_srli_epi16(v2, 1);
            __m128i v4 = _mm_or_si128(v1, v3);
            // _mm_blend_epi16 takes const int for parameter, so let's use blendv_epi8.
            m = _mm_blendv_epi8(m, v4, blender);
            m_[i] = FieldBits(m);
        }

        // both blender and exists are -1, puyo will drop 1.
        // -(-1) = +1
        dropAmount = _mm_sub_epi16(dropAmount, _mm_and_si128(blender, exists));
        exists = _mm_or_si128(exists, _mm_cmpeq_epi16(_mm_and_si128(line, whole), line));
    }

    // We have _mm_minpos_epu16, but not _mm_maxpos_epu16. So, taking xor 1.
    int maxDropAmountNegative = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_xor_si128(ones, dropAmount)));
    return ~maxDropAmountNegative & 0xFF;
}
