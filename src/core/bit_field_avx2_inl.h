#ifndef CORE_BIT_FIELD_AVX2_INL_256_H_
#define CORE_BIT_FIELD_AVX2_INL_256_H_

#if !defined(__AVX2__) || !defined(__BMI2__)
# error "Needs AVX2 and BMI2 to use this header."
#endif

#include "field_bits_256.h"

template<typename Tracker>
RensaResult BitField::simulateAVX2(SimulationContext* context, Tracker* tracker)
{
    // TODO(mayah): Write this.
    return simulate(context, tracker);
}

template<typename Tracker>
int BitField::simulateFastAVX2(Tracker* tracker)
{
    BitField escaped = escapeInvisible();
    int currentChain = 1;

    FieldBits erased;
    while (vanishFastAVX2(currentChain, &erased, tracker)) {
        currentChain += 1;
        dropAfterVanishFastBMI2(erased, tracker);
    }

    recoverInvisible(escaped);
    return currentChain - 1;
}

template<typename Tracker>
RensaStepResult BitField::vanishDropAVX2(SimulationContext* context, Tracker* tracker)
{
    // TODO(mayah): Write this.
    return vanishDrop(context, tracker);
}

template<typename Tracker>
bool BitField::vanishDropFastAVX2(SimulationContext* context, Tracker* tracker)
{
    BitField escaped = escapeInvisible();

    bool vanished = false;
    FieldBits erased;
    if (vanishFastAVX2(context->currentChain, &erased, tracker)) {
        dropAfterVanishFastBMI2(erased, tracker);
        context->currentChain += 1;
        vanished = true;
    }

    recoverInvisible(escaped);
    return vanished;
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
void BitField::dropAfterVanishFastBMI2(FieldBits erased, Tracker* tracker)
{
    union Decomposer64 {
        std::uint64_t v[2];
        __m128i m;
    };

    union Decomposer256_64 {
        std::uint64_t v[4];
        __m256i m;
    };

    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);
    Decomposer64 x;
    x.m = leftBits;
    const std::uint64_t oldLowBits = x.v[0];
    const std::uint64_t oldHighBits = x.v[1];

    const __m256i ones = _mm256_set_epi32(0, 1, 1, 1, 1, 1, 1, 0);
    __m256i height = _mm256_cvtepi16_epi32(sse::mm_popcnt_epi16(leftBits));
    height = _mm256_sllv_epi32(ones, height);
    height = _mm256_sub_epi32(height, ones);
    height = _mm256_slli_epi32(height, 1);

    height = _mm256_packs_epi32(height, height);
    Decomposer256_64 y;
    y.m = height;
    const std::uint64_t newLowBits = y.v[0];
    const std::uint64_t newHighBits = y.v[2];

    for (int i = 0; i < 3; ++i) {
        Decomposer64 d;
        d.m = m_[i];

        std::uint64_t extLow = _pext_u64(d.v[0], oldLowBits);
        std::uint64_t depLow = _pdep_u64(extLow, newLowBits);

        std::uint64_t extHigh = _pext_u64(d.v[1], oldHighBits);
        std::uint64_t depHigh = _pdep_u64(extHigh, newHighBits);

        d.v[0] = depLow;
        d.v[1] = depHigh;
        m_[i] = d.m;
    }

    tracker->trackDropBMI2(oldLowBits, oldHighBits, newLowBits, newHighBits);
}

#endif // CORE_BIT_FIELD_AVX2_INL_256_H_
