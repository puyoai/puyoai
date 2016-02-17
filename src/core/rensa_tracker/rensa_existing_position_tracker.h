#ifndef CORE_RENSA_TRACKER_RENSA_EXISTING_POSITION_TRACKER_H_
#define CORE_RENSA_TRACKER_RENSA_EXISTING_POSITION_TRACKER_H_

#if !defined(_MSC_VER)
#include <x86intrin.h>
#endif

#include "core/rensa_tracker.h"

class RensaExistingPositionTrackResult {
public:
    explicit RensaExistingPositionTrackResult(FieldBits bits) : existingBits_(bits) {}

    void setExistingBits(FieldBits bits) { existingBits_ = bits; }
    FieldBits existingBits() const { return existingBits_; }

private:
    FieldBits existingBits_;
};

template<>
class RensaTracker<RensaExistingPositionTrackResult> {
public:
    RensaTracker(FieldBits bits) : result_(bits) {}

    const RensaExistingPositionTrackResult& result() const { return result_; }

    void trackCoef(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/) {}
    void trackVanish(int /*nthChain*/, const FieldBits& /*vanishedPuyoBits*/, const FieldBits& /*vanishedOjamaPuyoBits*/) {}

    void trackDrop(FieldBits blender, FieldBits leftOnes, FieldBits rightOnes)
    {
        FieldBits m = result_.existingBits();

        __m128i v1 = _mm_and_si128(rightOnes, m);
        __m128i v2 = _mm_and_si128(leftOnes, m);
        __m128i v3 = _mm_srli_epi16(v2, 1);
        __m128i v4 = _mm_or_si128(v1, v3);
        m = _mm_blendv_epi8(m, v4, blender);

        result_.setExistingBits(m);
    }

#ifdef __BMI2__
    void trackDropBMI2(std::uint64_t oldLowBits, std::uint64_t oldHighBits, std::uint64_t newLowBits, std::uint64_t newHighBits)
    {
        union {
            std::uint64_t v[2];
            __m128i m;
        };

        m = result_.existingBits();
        v[0] = _pdep_u64(_pext_u64(v[0], oldLowBits), newLowBits);
        v[1] = _pdep_u64(_pext_u64(v[1], oldHighBits), newHighBits);
        result_.setExistingBits(m);
    }

#endif

private:
    RensaExistingPositionTrackResult result_;
};
typedef RensaTracker<RensaExistingPositionTrackResult> RensaExistingPositionTracker;

#endif // CORE_RENSA_TRACKER_RENSA_EXISTING_POSITION_TRACKER_H_
