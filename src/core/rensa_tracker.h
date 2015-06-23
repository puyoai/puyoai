#ifndef CORE_RENSA_TRACKER_H_
#define CORE_RENSA_TRACKER_H_

#include <glog/logging.h>

#include "base/bmi.h"
#include "base/unit.h"
#include "core/field_bits.h"
#include "core/rensa_result.h"
#include "core/rensa_track_result.h"

class FieldBits;

// RensaTracker tracks how rensa is vanished.
// For example, a puyo is vanished in what-th chain, coefficient of each chain, etc.
// You can pass a RensaTracker to CoreField::simulate() to track the rensa.
// You can also define you own RensaTracker, and pass it to CoreField::simulate().
//
// RensaTracker must define several interface. CoreField::simulate() has several hook poinits
// that calls the corresponding Tracker methods. If you'd like to add a new hook point,
// you need to define a hook point in CoreField.

class RensaTrackerBase {
public:
    void track(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/,
               const FieldBits& /*vanishedColorPuyoBits*/, const FieldBits& /*vanishedOjamaPuyoBits*/) {}

    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}
};

// ----------------------------------------------------------------------

template<typename TrackResult>
class RensaTracker;

// ----------------------------------------------------------------------

// RensaTracker<Unit> is a tracker that does not track anything.
template<>
class RensaTracker<Unit> : public RensaTrackerBase {
public:
};
typedef RensaTracker<Unit> RensaNonTracker;

// ----------------------------------------------------------------------

class RensaYPositionTracker : public RensaTrackerBase {
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

    void track(int /*nthChain*/, int /*numErasedPuyo*/, int /*longBonusCoef*/, int /*colorBonusCoef*/,
               const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
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

    int originalY(int x, int y) const { return (originalY_[x] >> (4 * y)) & 0xF; }

private:
    std::uint64_t originalY_[FieldConstant::MAP_WIDTH];
};

// ----------------------------------------------------------------------

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

// ----------------------------------------------------------------------

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

// ----------------------------------------------------------------------

// RensaTracker<RensaChainTrackResult> tracks in what-th rensa a puyo is vanished.
template<>
class RensaTracker<RensaChainTrackResult> : public RensaTrackerBase {
public:
    RensaTracker() {}

    const RensaChainTrackResult& result() const { return result_; }

    void track(int nthChain, int numErasedPuyo, int longBonusCoef, int colorBonusCoef,
               const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        FieldBits m = (vanishedColorPuyoBits | vanishedOjamaPuyoBits);
        m.iterateBitPositions([&](int x, int y) {
            result_.setErasedAt(x, yTracker_.originalY(x, y), nthChain);
        });

        yTracker_.track(nthChain, numErasedPuyo, longBonusCoef, colorBonusCoef, vanishedColorPuyoBits, vanishedOjamaPuyoBits);
    }

private:
    RensaYPositionTracker yTracker_;
    RensaChainTrackResult result_;
};
typedef RensaTracker<RensaChainTrackResult> RensaChainTracker;

// ----------------------------------------------------------------------

template<>
class RensaTracker<RensaCoefResult> : public RensaTrackerBase {
public:
    const RensaCoefResult& result() const { return result_; }

    void track(int nthChain, int numErasedPuyo, int longBonusCoef, int colorBonusCoef,
               const FieldBits& /*vanishedColorPuyoBits*/, const FieldBits& /*vanishedOjamaPuyoBits*/)
    {
        result_.setCoef(nthChain, numErasedPuyo, longBonusCoef, colorBonusCoef);
    }

private:
    RensaCoefResult result_;
};
typedef RensaTracker<RensaCoefResult> RensaCoefTracker;

// ----------------------------------------------------------------------

template<>
class RensaTracker<RensaVanishingPositionResult> : public RensaTrackerBase {
public:
    RensaTracker() {}

    const RensaVanishingPositionResult& result() const { return result_; }

    void track(int nthChain, int numErasedPuyo, int longBonusCoef, int colorBonusCoef,
               const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        vanishedColorPuyoBits.iterateBitPositions([&](int x, int y) {
            if (yTracker_.originalY(x, y) == y) {
                result_.setBasePuyo(x, y, nthChain);
            } else {
                result_.setFallingPuyo(x, yTracker_.originalY(x, y), y, nthChain);
            }
        });

        yTracker_ = RensaYPositionTracker();
        yTracker_.track(nthChain, numErasedPuyo, longBonusCoef, colorBonusCoef, vanishedColorPuyoBits, vanishedOjamaPuyoBits);
    }

private:
    RensaVanishingPositionResult result_;
    RensaYPositionTracker yTracker_;
};
typedef RensaTracker<RensaVanishingPositionResult> RensaVanishingPositionTracker;

// ----------------------------------------------------------------------

// This is the same as RensaChainTracker, however, the result is passed as pointer.
class RensaChainPointerTracker : public RensaTrackerBase {
public:
    explicit RensaChainPointerTracker(RensaChainTrackResult* trackResult) :
        result_(trackResult)
    {
        // TODO(mayah): Assert trackResult is initialized?
    }

    const RensaChainTrackResult& result() const { return *result_; }

    void track(int nthChain, int numErasedPuyo, int longBonusCoef, int colorBonusCoef,
               const FieldBits& vanishedColorPuyoBits, const FieldBits& vanishedOjamaPuyoBits)
    {
        FieldBits m = (vanishedColorPuyoBits | vanishedOjamaPuyoBits);
        m.iterateBitPositions([&](int x, int y) {
            result_->setErasedAt(x, yTracker_.originalY(x, y), nthChain);
        });

        yTracker_.track(nthChain, numErasedPuyo, longBonusCoef, colorBonusCoef, vanishedColorPuyoBits, vanishedOjamaPuyoBits);
    }

private:
    RensaYPositionTracker yTracker_;
    RensaChainTrackResult* result_;
};

#endif
