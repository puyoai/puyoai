#ifndef CORE_BIT_FIELD_AVX2_INL_256_H_
#define CORE_BIT_FIELD_AVX2_INL_256_H_

#ifndef __AVX2__
# error "Needs AVX2 to use this header."
#endif

#include "field_bits_256.h"

inline int BitField::simulateFastAVX2()
{
    BitField escaped = escapeInvisible();
    int currentChain = 1;

    FieldBits erased;
    while (vanishFastAVX2(currentChain, &erased)) {
        currentChain += 1;
#ifdef __BMI2__
        dropAfterVanishFastBMI2(erased);
#else
        RensaNonTracker tracker;
        dropAfterVanishFast(erased, &tracker);
#endif
    }

    recoverInvisible(escaped);
    return currentChain - 1;
}

inline
bool BitField::vanishFastAVX2(int /*currentChain*/, FieldBits* erased) const
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

    return true;
}

#endif // CORE_BIT_FIELD_AVX2_INL_256_H_
