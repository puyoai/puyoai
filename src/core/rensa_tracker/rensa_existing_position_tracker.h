#ifndef CORE_RENSA_TRACKER_RENSA_EXISTING_POSITION_TRACKER_H_
#define CORE_RENSA_TRACKER_RENSA_EXISTING_POSITION_TRACKER_H_

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
class RensaTracker<RensaExistingPositionTrackResult> : public RensaTrackerBase {
public:
    RensaTracker(FieldBits bits) : result_(bits) {}

    const RensaExistingPositionTrackResult& result() const { return result_; }

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

private:
    RensaExistingPositionTrackResult result_;
};
typedef RensaTracker<RensaExistingPositionTrackResult> RensaExistingPositionTracker;

#endif // CORE_RENSA_TRACKER_RENSA_EXISTING_POSITION_TRACKER_H_
