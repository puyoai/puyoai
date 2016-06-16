#ifndef CPU_PERIA_RENSA_VANISHING_POSITION_TRACKER_H_
#define CPU_PERIA_RENSA_VANISHING_POSITION_TRACKER_H_

// TODO(peria): Move this tracker, replacing with RensaVanishingPositionTracker.
// The original RenaVanishingPositionTracker must be renamed.

#include <array>
#include <vector>

#include "core/field_bits.h"
#include "core/rensa_tracker.h"
#include "core/rensa_tracker/rensa_yposition_tracker.h"

// This tracks puyo position, where the puyo vanishes at n-th chain.
class VanishingPositionTrackerResult {
public:
    VanishingPositionTrackerResult() {
        basePuyosErasedAt_.reserve(19);
    }

    int size() const {
        return static_cast<int>(basePuyosErasedAt_.size());
    }
  
    // where the puyos vanished at n-th chain.
    const FieldBits& getBasePuyosAt(int nthChain) const {
        DCHECK_GE(nthChain, 1);
        return basePuyosErasedAt_[nthChain - 1];
    }

    void setBasePuyosAt(const FieldBits& bits, int nthChain) {
        if ((int) basePuyosErasedAt_.size() < nthChain)
            basePuyosErasedAt_.resize(nthChain);
        basePuyosErasedAt_[nthChain - 1] = bits;
    }

private:
    std::vector<FieldBits> basePuyosErasedAt_;
};

template<>
class RensaTracker<VanishingPositionTrackerResult> {
public:
    RensaTracker() {}

    const VanishingPositionTrackerResult& result() const { return result_; }

    void trackCoef(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/) {}
    void trackVanish(int nthChain, const FieldBits& vanishedPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        FieldBits vanishedColorPuyoBits = vanishedPuyoBits.notmask(vanishedOjamaPuyoBits);
        result_.setBasePuyosAt(vanishedColorPuyoBits, nthChain);
    }

    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}
#ifdef __BMI2__
    void trackDropBMI2(std::uint64_t /*oldLowBits*/, std::uint64_t /*oldHighBits*/, std::uint64_t /*newLowBits*/, std::uint64_t /*newHighBits*/) {}
#endif

private:
    VanishingPositionTrackerResult result_;
};

using RensaVanishingPositionTracker = RensaTracker<VanishingPositionTrackerResult>;

#endif // CPU_PERIA_RENSA_VANISHING_POSITION_TRACKER_H_
