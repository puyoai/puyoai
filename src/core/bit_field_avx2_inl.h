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
        dropFastAfterVanishBMI2(erased);
#else
        RensaNonTracker tracker;
        dropFastAfterVanish(erased, &tracker);
#endif
    }

    recoverInvisible(escaped);
    return currentChain - 1;
}

inline
bool BitField::vanishFastAVX2(int /*currentChain*/, FieldBits* erased) const
{
    *erased = FieldBits();

    bool didErase = false;

    // RED & BLUE
    {
        // TODO(mayah): This can be improved.
        FieldBits256 mask(bits(PuyoColor::RED).maskedField12(), bits(PuyoColor::BLUE).maskedField12());
        FieldBits256 seed = mask.vanishingSeed();
        if (!seed.isEmpty()) {
            FieldBits256 expanded = seed.expand(mask);
            // TODO(mayah): This can be improved.
            erased->setAll(expanded.low());
            erased->setAll(expanded.high());
            didErase = true;
        }
    }

    // YELLOW & GREEN
    {
        // TODO(mayah): This can be improved.
        FieldBits256 mask(bits(PuyoColor::YELLOW).maskedField12(), bits(PuyoColor::GREEN).maskedField12());
        FieldBits256 seed = mask.vanishingSeed();
        if (!seed.isEmpty()) {
            FieldBits256 expanded = seed.expand(mask);
            // TODO(mayah): This can be improved.
            erased->setAll(expanded.low());
            erased->setAll(expanded.high());
            didErase = true;
        }
    }

    if (!didErase)
        return false;

    // Removes ojama.
    FieldBits ojamaErased(erased->expandEdge().mask(bits(PuyoColor::OJAMA).maskedField12()));
    erased->setAll(ojamaErased);

    return true;
}

#endif // CORE_BIT_FIELD_AVX2_INL_256_H_
