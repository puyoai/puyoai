#ifndef CORE_RENSA_TRACKER_RENSA_VANISHING_POSITION_TRACKER_H_
#define CORE_RENSA_TRACKER_RENSA_VANISHING_POSITION_TRACKER_H_

#include <array>
#include <vector>

#include "core/rensa_tracker.h"
#include "core/rensa_tracker/rensa_yposition_tracker.h"

// This tracks puyo position at "n-1"-th chain, where the puyo vanishes at n-th chain.
class RensaVanishingPositionResult {
public:
    RensaVanishingPositionResult() {
        basePuyosErasedAt_.reserve(19);
        fallingPuyosErasedAt_.reserve(19);
    }

    int size() const {
        return fallingPuyosErasedAt_.size();
    }

    // Gets the reference of position set, where the puyos vanish after falling at n-th chain.
    // The return value is valid while this result instance is valid.
    const std::vector<Position>& getReferenceFallingPuyosAt(int nthChain) const {
        return fallingPuyosErasedAt_[nthChain - 1];
    }

    // Gets the reference of position set, where the puyos vanish without falling at n-th chain.
    // The return value is valid while this result instance is valid.
    const std::vector<Position>& getReferenceBasePuyosAt(int nthChain) const {
        return basePuyosErasedAt_[nthChain - 1];
    }

    std::array<float, 2> getWeightedCenterAfterFall(int nthChain) const;

    void setFallingPuyo(int x, int yBeforeFall, int yAfterFall, int nthChain);
    void setBasePuyo(int x, int y, int nthChain);

private:
    std::vector<std::vector<Position>> basePuyosErasedAt_;
    std::vector<std::vector<Position>> fallingPuyosErasedAt_;
    std::vector<std::vector<int>> yOfFalledPuyosErasedAt_;
    void maybeResize(int nthChain);
};

template<>
class RensaTracker<RensaVanishingPositionResult> {
public:
    RensaTracker() {}

    const RensaVanishingPositionResult& result() const { return result_; }

    void trackCoef(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/) {}
    void trackVanish(int nthChain, const FieldBits& vanishedPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        FieldBits vanishedColorPuyoBits = vanishedPuyoBits.notmask(vanishedOjamaPuyoBits);
        vanishedColorPuyoBits.iterateBitPositions([&](int x, int y) {
            if (yTracker_.originalY(x, y) == y) {
                result_.setBasePuyo(x, y, nthChain);
            } else {
                result_.setFallingPuyo(x, yTracker_.originalY(x, y), y, nthChain);
            }
        });

        yTracker_ = RensaYPositionTracker();
        yTracker_.trackVanish(nthChain, vanishedColorPuyoBits, vanishedOjamaPuyoBits);
    }

    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}
#ifdef __BMI2__
    void trackDropBMI2(std::uint64_t /*oldLowBits*/, std::uint64_t /*oldHighBits*/, std::uint64_t /*newLowBits*/, std::uint64_t /*newHighBits*/) {}
#endif

private:
    RensaVanishingPositionResult result_;
    RensaYPositionTracker yTracker_;
};
typedef RensaTracker<RensaVanishingPositionResult> RensaVanishingPositionTracker;

#endif // CORE_RENSA_TRACKER_RENSA_VANISHING_POSITION_TRACKER_H_
