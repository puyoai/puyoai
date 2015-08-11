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
class RensaTracker<RensaLastVanishedPositionTrackResult> : public RensaTrackerBase {
public:
    const RensaLastVanishedPositionTrackResult& result() const { return result_; }

    void track(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/,
               const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        result_.setLastVanishedPositionBits(vanishedColorPuyoBits | vanishedOjamaPuyoBits);
    }

private:
    RensaLastVanishedPositionTrackResult result_;
};
typedef RensaTracker<RensaLastVanishedPositionTrackResult> RensaLastVanishedPositionTracker;

#endif // CORE_RENSA_TRACKER_RENSA_LAST_VANISHED_POSITION_TRACKER_H_
