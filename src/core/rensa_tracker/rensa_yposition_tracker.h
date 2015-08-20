#ifndef CORE_RENSA_RENSA_YPOSITION_TRACKER_H_
#define CORE_RENSA_RENSA_YPOSITION_TRACKER_H_

#include "base/bmi.h"
#include "core/field_constant.h"
#include "core/rensa_tracker.h"

class RensaYPositionTracker {
public:
    RensaYPositionTracker() :
        originalY_ {
            0xFEDCBA9876543210,
            0xFEDCBA9876543210,
            0xFEDCBA9876543210,
            0xFEDCBA9876543210,
            0xFEDCBA9876543210,
            0xFEDCBA9876543210,
            0xFEDCBA9876543210,
            0xFEDCBA9876543210,
        }
    {
    }

    int originalY(int x, int y) const { return (originalY_[x] >> (4 * y)) & 0xF; }

    void trackCoef(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/) {}

    void trackVanish(int /*nthChain*/, const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        const __m128i zero = _mm_setzero_si128();
        const __m128i ones = _mm_cmpeq_epi8(zero, zero);
        union {
            std::uint16_t cols[FieldConstant::MAP_WIDTH];
            __m128i m;
        };
        m = (vanishedColorPuyoBits | vanishedOjamaPuyoBits) ^ ones;

        for (int x = 1; x <= 6; ++x) {
            originalY_[x] = bmi::extractBits4(originalY_[x], cols[x]);
        }
    }

    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}


private:
    std::uint64_t originalY_[FieldConstant::MAP_WIDTH];
};

#endif // CORE_RENSA_RENSA_YPOSITION_TRACKER_H_
