#ifndef CORE_RENSA_TRACKER_RENSA_CHAIN_TRACKER_H_
#define CORE_RENSA_TRACKER_RENSA_CHAIN_TRACKER_H_

#include <string>

#include "core/field_constant.h"
#include "core/rensa_tracker.h"
#include "core/rensa_tracker/rensa_yposition_tracker.h"

// RensaChainTrackResult represents in what-th chain puyo is erased.
class RensaChainTrackResult {
public:
    RensaChainTrackResult();
    explicit RensaChainTrackResult(const std::string&);

    RensaChainTrackResult& operator=(const RensaChainTrackResult& result);

    // Nth Rensa where (x, y) is erased. 0 if not erased.
    int erasedAt(int x, int y) const { return erasedAt_[x][y]; }
    void setErasedAt(int x, int y, int nthChain) { erasedAt_[x][y] = nthChain; }

    std::string toString() const;

private:
    std::uint8_t erasedAt_[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT];
};

// RensaTracker<RensaChainTrackResult> tracks in what-th rensa a puyo is vanished.
template<>
class RensaTracker<RensaChainTrackResult> {
public:
    RensaTracker() {}

    const RensaChainTrackResult& result() const { return result_; }

    void trackCoef(int nthChain, int numErasedPuyo, int longBonusCoef, int colorBonusCoef)
    {
        yTracker_.trackCoef(nthChain, numErasedPuyo, longBonusCoef, colorBonusCoef);
    }

    void trackVanish(int nthChain, const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        FieldBits m = (vanishedColorPuyoBits | vanishedOjamaPuyoBits);
        m.iterateBitPositions([&](int x, int y) {
            result_.setErasedAt(x, yTracker_.originalY(x, y), nthChain);
        });

        yTracker_.trackVanish(nthChain, vanishedColorPuyoBits, vanishedOjamaPuyoBits);
    }

    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}
#ifdef __BMI2__
    void trackDropBMI2(std::uint64_t /*oldLowBits*/, std::uint64_t /*oldHighBits*/, std::uint64_t /*newLowBits*/, std::uint64_t /*newHighBits*/) {}
#endif

private:
    RensaYPositionTracker yTracker_;
    RensaChainTrackResult result_;
};
typedef RensaTracker<RensaChainTrackResult> RensaChainTracker;

// This is the same as RensaChainTracker, however, the result is passed as pointer.
class RensaChainPointerTracker {
public:
    explicit RensaChainPointerTracker(RensaChainTrackResult* trackResult) :
        result_(trackResult)
    {
        // TODO(mayah): Assert trackResult is initialized?
    }

    const RensaChainTrackResult& result() const { return *result_; }

    void trackCoef(int nthChain, int numErasedPuyo, int longBonusCoef, int colorBonusCoef)
    {
        yTracker_.trackCoef(nthChain, numErasedPuyo, longBonusCoef, colorBonusCoef);
    }

    void trackVanish(int nthChain, const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        FieldBits m = (vanishedColorPuyoBits | vanishedOjamaPuyoBits);
        m.iterateBitPositions([&](int x, int y) {
            result_->setErasedAt(x, yTracker_.originalY(x, y), nthChain);
        });

        yTracker_.trackVanish(nthChain, vanishedColorPuyoBits, vanishedOjamaPuyoBits);
    }

    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}
#ifdef __BMI2__
    void trackDropBMI2(std::uint64_t /*oldLowBits*/, std::uint64_t /*oldHighBits*/, std::uint64_t /*newLowBits*/, std::uint64_t /*newHighBits*/) {}
#endif

private:
    RensaYPositionTracker yTracker_;
    RensaChainTrackResult* result_;
};

#endif
