#ifndef CORE_BIT_FIELD_AVX2_INL_256_H_
#define CORE_BIT_FIELD_AVX2_INL_256_H_

#if !defined(__AVX2__) || !defined(__BMI2__)
# error "Needs AVX2 and BMI2 to use this header."
#endif

#include "base/avx.h"
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
int BitField::vanishAVX2(int currentChain, FieldBits* erased, Tracker* tracker) const
{
    // TODO(mayah): write this.
    return vanish(currentChain, erased, tracker);
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
int BitField::dropAfterVanishAVX2(FieldBits erased, Tracker* tracker)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);

    sse::Decomposer t;

    int maxDrops = 0;
    t.m = (m_[0] | m_[1] | m_[2]).notmask(erased);
    for (int x = 1; x <= 6; ++x) {
        if (t.ui16[x] == 0)
            continue;
        int h = 31 - __builtin_clz(t.ui16[x]);
        int p = __builtin_popcount(t.ui16[x] ^ (((1 << h) - 1) << 1));
        maxDrops = std::max(p, maxDrops);
    }

    t.m = leftBits;
    const std::uint64_t oldLowBits = t.ui64[0];
    const std::uint64_t oldHighBits = t.ui64[1];

    const __m256i ones = _mm256_set_epi32(0, 1, 1, 1, 1, 1, 1, 0);
    __m256i height = _mm256_cvtepi16_epi32(sse::mm_popcnt_epi16(leftBits));
    height = _mm256_sllv_epi32(ones, height);
    height = _mm256_sub_epi32(height, ones);
    height = _mm256_slli_epi32(height, 1);

    height = _mm256_packs_epi32(height, height);
    avx::Decomposer256 y;
    y.m = height;
    const std::uint64_t newLowBits = y.ui64[0];
    const std::uint64_t newHighBits = y.ui64[2];

    for (int i = 0; i < 3; ++i) {
        sse::Decomposer d;
        d.m = m_[i];
        d.ui64[0] = _pdep_u64(_pext_u64(d.ui64[0], oldLowBits), newLowBits);
        d.ui64[1] = _pdep_u64(_pext_u64(d.ui64[1], oldHighBits), newHighBits);
        m_[i] = d.m;
    }

    tracker->trackDropBMI2(oldLowBits, oldHighBits, newLowBits, newHighBits);

    return maxDrops;
}

template<typename Tracker>
void BitField::dropAfterVanishFastAVX2(FieldBits erased, Tracker* tracker)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);

    sse::Decomposer t;
    t.m = leftBits;
    const std::uint64_t oldLowBits = t.ui64[0];
    const std::uint64_t oldHighBits = t.ui64[1];

    const __m256i ones = _mm256_set_epi32(0, 1, 1, 1, 1, 1, 1, 0);
    __m256i height = _mm256_cvtepi16_epi32(sse::mm_popcnt_epi16(leftBits));
    height = _mm256_sllv_epi32(ones, height);
    height = _mm256_sub_epi32(height, ones);
    height = _mm256_slli_epi32(height, 1);

    height = _mm256_packs_epi32(height, height);
    avx::Decomposer256 y;
    y.m = height;
    const std::uint64_t newLowBits = y.ui64[0];
    const std::uint64_t newHighBits = y.ui64[2];

    for (int i = 0; i < 3; ++i) {
        sse::Decomposer d;
        d.m = m_[i];
        d.ui64[0] = _pdep_u64(_pext_u64(d.ui64[0], oldLowBits), newLowBits);
        d.ui64[1] = _pdep_u64(_pext_u64(d.ui64[1], oldHighBits), newHighBits);
        m_[i] = d.m;
    }

    tracker->trackDropBMI2(oldLowBits, oldHighBits, newLowBits, newHighBits);
}

#endif // CORE_BIT_FIELD_AVX2_INL_256_H_
