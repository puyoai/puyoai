#ifndef CORE_RENSA_TRACKER_RENSA_LAST_VANISHED_POSITION_TRACKER_H_
#define CORE_RENSA_TRACKER_RENSA_LAST_VANISHED_POSITION_TRACKER_H_

#include "core/rensa_tracker.h"

class RensaLastVanishedPositionTrackResult {
public:
    void setLastVanishedPositionBits(FieldBits bits) { lastVanishedPositionBits_ = bits; }
    FieldBits lastVanishedPositionBits() const { return lastVanishedPositionBits_; }
private:
    FieldBits lastVanishedPositionBits_;
};

template<>
class RensaTracker<RensaLastVanishedPositionTrackResult> {
public:
    const RensaLastVanishedPositionTrackResult& result() const { return result_; }

    void trackVanish(int /*nthChain*/, const FieldBits& vanishedPuyoBits, const FieldBits& /*vanishedOjamaPuyoBits*/)
    {
        result_.setLastVanishedPositionBits(vanishedPuyoBits);
    }

    void trackCoef(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/) {}
    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}
#ifdef __BMI2__
    void trackDropBMI2(std::uint64_t /*oldLowBits*/, std::uint64_t /*oldHighBits*/, std::uint64_t /*newLowBits*/, std::uint64_t /*newHighBits*/) {}
#endif

private:
    RensaLastVanishedPositionTrackResult result_;
};
typedef RensaTracker<RensaLastVanishedPositionTrackResult> RensaLastVanishedPositionTracker;

#endif // CORE_RENSA_TRACKER_RENSA_LAST_VANISHED_POSITION_TRACKER_H_
