#ifndef CORE_BIT_FIELD_AVX2_INL_256_H_
#define CORE_BIT_FIELD_AVX2_INL_256_H_

#if !defined(__AVX2__) || !defined(__BMI2__)
# error "Needs AVX2 and BMI2 to use this header."
#endif

#if !defined(_MSC_VER)
#include <x86intrin.h>
#endif

#include "base/avx.h"
#include "base/bmi.h"
#include "base/sse.h"
#include "field_bits_256.h"

template<typename Tracker>
RensaResult BitField::simulateAVX2(SimulationContext* context, Tracker* tracker)
{
    BitField escaped = escapeInvisible();

    int score = 0;
    int frames = 0;
    int nthChainScore;
    bool quick = false;
    FieldBits erased;

    while ((nthChainScore = vanishAVX2(context->currentChain, &erased, tracker)) > 0) {
        context->currentChain += 1;
        score += nthChainScore;
        frames += FRAMES_VANISH_ANIMATION;
        int maxDrops = dropAfterVanishAVX2(erased, tracker);
        if (maxDrops > 0) {
            frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
        } else {
            quick = true;
        }
    }

    recoverInvisible(escaped);
    return RensaResult(context->currentChain - 1, score, frames, quick);
}

template<typename Tracker>
int BitField::simulateFastAVX2(Tracker* tracker)
{
    BitField escaped = escapeInvisible();
    int currentChain = 1;

    FieldBits erased;
    while (vanishFastAVX2(currentChain, &erased, tracker)) {
        currentChain += 1;
        dropAfterVanishFastAVX2(erased, tracker);
    }

    recoverInvisible(escaped);
    return currentChain - 1;
}

template<typename Tracker>
RensaStepResult BitField::vanishDropAVX2(SimulationContext* context, Tracker* tracker)
{
    BitField escaped = escapeInvisible();

    FieldBits erased;
    int score = vanishAVX2(context->currentChain, &erased, tracker);
    int maxDrops = 0;
    int frames = FRAMES_VANISH_ANIMATION;
    bool quick = false;
    if (score > 0) {
        maxDrops = dropAfterVanishAVX2(erased, tracker);
        context->currentChain += 1;
    }

    if (maxDrops > 0) {
        DCHECK(maxDrops < 14);
        frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
    } else {
        quick = true;
    }

    recoverInvisible(escaped);
    return RensaStepResult(score, frames, quick);
}

template<typename Tracker>
bool BitField::vanishDropFastAVX2(SimulationContext* context, Tracker* tracker)
{
    BitField escaped = escapeInvisible();

    bool vanished = false;
    FieldBits erased;
    if (vanishFastAVX2(context->currentChain, &erased, tracker)) {
        dropAfterVanishFastAVX2(erased, tracker);
        context->currentChain += 1;
        vanished = true;
    }

    recoverInvisible(escaped);
    return vanished;
}

template<typename Tracker>
CLANG_ALWAYS_INLINE
int BitField::vanishAVX2(int currentChain, FieldBits* erased, Tracker* tracker) const
{
    FieldBits256 erased256;
    int numErasedPuyos = 0;
    int numColors = 0;
    int longBonusCoef = 0;

    bool didErase = false;

    for (int i = 0; i < 2; ++i) {
        FieldBits t = (i == 0) ? FieldBits(_mm_andnot_si128(m_[1], m_[2])) : m_[2] & m_[1];
        t = t.maskedField12();

        FieldBits highMask = m_[0] & t;
        FieldBits lowMask = _mm_andnot_si128(m_[0], t);

        FieldBits256 mask(highMask, lowMask);
        FieldBits256 vanishing;
        if (!mask.findVanishingBits(&vanishing))
            continue;
        erased256.setAll(vanishing);
        didErase = true;

        std::pair<int, int> pc = vanishing.popcountHighLow();
        int highCount = pc.first;
        int lowCount = pc.second;

        if (highCount > 0) {
            ++numColors;
            numErasedPuyos += highCount;
            if (highCount <= 7) {
                longBonusCoef += longBonus(highCount);
            } else {
                FieldBits high = vanishing.high();
                // slowpath
                high.iterateBitWithMasking([&](FieldBits x) -> FieldBits {
                    FieldBits expanded = x.expand(highMask);
                    longBonusCoef += longBonus(expanded.popcount());
                    return expanded;
                });
            }
        }

        if (lowCount > 0) {
            ++numColors;
            numErasedPuyos += lowCount;
            if (lowCount <= 7) {
                longBonusCoef += longBonus(lowCount);
            } else {
                FieldBits low = vanishing.low();
                low.iterateBitWithMasking([&](FieldBits x) -> FieldBits {
                    FieldBits expanded = x.expand(lowMask);
                    longBonusCoef += longBonus(expanded.popcount());
                    return expanded;
                });
            }
        }
    }

    if (!didErase) {
        *erased = FieldBits();
        return false;
    }

    *erased = erased256.low() | erased256.high();

    int colorBonusCoef = colorBonus(numColors);
    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(currentChain), longBonusCoef, colorBonusCoef);
    tracker->trackCoef(currentChain, numErasedPuyos, longBonusCoef, colorBonusCoef);

    // Removes ojama.
    FieldBits ojamaErased(erased->expandEdge().mask(bits(PuyoColor::OJAMA).maskedField12()));
    erased->setAll(ojamaErased);
    tracker->trackVanish(currentChain, *erased, ojamaErased);

    return 10 * numErasedPuyos * rensaBonusCoef;
}

template<typename Tracker>
bool BitField::vanishFastAVX2(int currentChain, FieldBits* erased, Tracker* tracker) const
{
    FieldBits256 erased256;

    bool didErase = false;

    // RED (100) & BLUE (101)
    {
        FieldBits t = _mm_andnot_si128(m_[1], m_[2]);
        t = t.maskedField12();
        FieldBits256 mask(m_[0] & t, _mm_andnot_si128(m_[0], t));
        FieldBits256 vanishing;
        if (mask.findVanishingBits(&vanishing)) {
            erased256.setAll(vanishing);
            didErase = true;
        }
    }

    // YELLOW (110) & GREEN (111)
    {
        FieldBits t = m_[2] & m_[1];
        t = t.maskedField12();
        FieldBits256 mask(m_[0] & t, _mm_andnot_si128(m_[0], t));
        FieldBits256 vanishing;
        if (mask.findVanishingBits(&vanishing)) {
            erased256.setAll(vanishing);
            didErase = true;
        }
    }

    if (!didErase) {
        *erased = FieldBits();
        return false;
    }

    *erased = erased256.low() | erased256.high();

    // Removes ojama.
    FieldBits ojamaErased(erased->expandEdge().mask(bits(PuyoColor::OJAMA).maskedField12()));
    erased->setAll(ojamaErased);

    tracker->trackVanish(currentChain, *erased, ojamaErased);

    return true;
}

template<typename Tracker>
CLANG_ALWAYS_INLINE
int BitField::dropAfterVanishAVX2(FieldBits erased, Tracker* tracker)
{
    // Set 1 at non-empty position.
    __m128i nonempty = (m_[0] | m_[1] | m_[2]).xmm();
    // Remove 1 bits from the positions where they are  erased.
    nonempty = _mm_andnot_si128(erased, nonempty);

    // Find the holes. The number of holes for each column is the number of
    // drops of the column.
    __m128i holes = _mm_and_si128(sse::mm_porr_epi16(nonempty), erased);
    __m128i num_holes = sse::mm_popcnt_epi16(holes);
    int maxDrops = sse::mm_hmax_epu16(num_holes);

    dropAfterVanishFastAVX2(erased, tracker);

    return maxDrops;
}

template<typename Tracker>
void BitField::dropAfterVanishFastAVX2(FieldBits erased, Tracker* tracker)
{
    const __m128i ones = sse::mm_setone_si128();

    sse::Decomposer t;
    t.m = _mm_xor_si128(erased, ones);
    const std::uint64_t oldLowBits = t.ui64[0];
    const std::uint64_t oldHighBits = t.ui64[1];

    __m256i shift = _mm256_cvtepu16_epi32(sse::mm_popcnt_epi16(erased));
    __m256i halfOnes = _mm256_cvtepu16_epi32(ones);
    __m256i shifted = _mm256_srlv_epi32(halfOnes, shift);
    shifted = _mm256_packus_epi32(shifted, shifted);

    avx::Decomposer256 y;
    y.m = shifted;
    const std::uint64_t newLowBits = y.ui64[0];
    const std::uint64_t newHighBits = y.ui64[2];

    sse::Decomposer d[3];
    d[0].m = m_[0];
    d[1].m = m_[1];
    d[2].m = m_[2];

    if (newLowBits != 0xFFFFFFFFFFFFFFFFULL) {
        d[0].ui64[0] = _pdep_u64(_pext_u64(d[0].ui64[0], oldLowBits), newLowBits);
        d[1].ui64[0] = _pdep_u64(_pext_u64(d[1].ui64[0], oldLowBits), newLowBits);
        d[2].ui64[0] = _pdep_u64(_pext_u64(d[2].ui64[0], oldLowBits), newLowBits);

        if (newHighBits != 0xFFFFFFFFFFFFFFFFULL) {
            d[0].ui64[1] = _pdep_u64(_pext_u64(d[0].ui64[1], oldHighBits), newHighBits);
            d[1].ui64[1] = _pdep_u64(_pext_u64(d[1].ui64[1], oldHighBits), newHighBits);
            d[2].ui64[1] = _pdep_u64(_pext_u64(d[2].ui64[1], oldHighBits), newHighBits);
        }
    } else {
        d[0].ui64[1] = _pdep_u64(_pext_u64(d[0].ui64[1], oldHighBits), newHighBits);
        d[1].ui64[1] = _pdep_u64(_pext_u64(d[1].ui64[1], oldHighBits), newHighBits);
        d[2].ui64[1] = _pdep_u64(_pext_u64(d[2].ui64[1], oldHighBits), newHighBits);
    }

    m_[0] = d[0].m;
    m_[1] = d[1].m;
    m_[2] = d[2].m;

    tracker->trackDropBMI2(oldLowBits, oldHighBits, newLowBits, newHighBits);
}

#endif // CORE_BIT_FIELD_AVX2_INL_256_H_
