#ifndef CORE_RENSA_TRACKER_H_
#define CORE_RENSA_TRACKER_H_

#include <glog/logging.h>

#include "base/bmi.h"
#include "base/unit.h"
#include "core/rensa_result.h"
#include "core/rensa_track_result.h"

// RensaTracker tracks how rensa is vanished.
// For example, a puyo is vanished in what-th chain, coefficient of each chain, etc.
// You can pass a RensaTracker to CoreField::simulate() to track the rensa.
// You can also define you own RensaTracker, and pass it to CoreField::simulate().
//
// RensaTracker must define several interface. CoreField::simulate() has several hook poinits
// that calls the corresponding Tracker methods. If you'd like to add a new hook point,
// you need to define a hook point in CoreField.

template<typename TrackResult>
class RensaTracker;

// RensaTracker<Unit> is a tracker that does not track anything.
template<>
class RensaTracker<Unit> {
public:
    void track(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/,
               const FieldBits& /*vanishedColorPuyoBits*/, const FieldBits& /*vanishedOjamaPuyoBits*/) {}

    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) { }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) {}
};
typedef RensaTracker<Unit> RensaNonTracker;

class RensaYPositionTracker {
public:
    RensaYPositionTracker() :
        originalY_ {
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
        }
    {
    }

    int originalY(int x, int y) const { return originalY_[x][y]; }

    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void puyoIsDropped(int x, int fromY, int toY)
    {
        DCHECK_NE(fromY, toY);
        originalY_[x][toY] = originalY_[x][fromY];
        originalY_[x][fromY] = 0;
    }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) {}

private:
    int originalY_[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT];
};

class BitRensaYPositionTracker {
public:
    BitRensaYPositionTracker() :
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

    void track(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/,
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

// RensaTracker<RensaChainTrackResult> tracks in what-th rensa a puyo is vanished.
template<>
class RensaTracker<RensaChainTrackResult> {
public:
    RensaTracker() {}

    const RensaChainTrackResult& result() const { return result_; }

    void colorPuyoIsVanished(int x, int y, int nthChain)
    {
        yTracker_.colorPuyoIsVanished(x, y, nthChain);
        result_.setErasedAt(x, yTracker_.originalY(x, y), nthChain);
    }
    void ojamaPuyoIsVanished(int x, int y, int nthChain)
    {
        yTracker_.ojamaPuyoIsVanished(x, y, nthChain);
        result_.setErasedAt(x, yTracker_.originalY(x, y), nthChain);
    }
    void puyoIsDropped(int x, int fromY, int toY)
    {
        yTracker_.puyoIsDropped(x, fromY, toY);
    }
    void nthChainDone(int nthChain, int numErasedPuyo, int coef)
    {
        yTracker_.nthChainDone(nthChain, numErasedPuyo, coef);
    }

private:
    RensaYPositionTracker yTracker_;
    RensaChainTrackResult result_;
};
typedef RensaTracker<RensaChainTrackResult> RensaChainTracker;

template<>
class RensaTracker<RensaCoefResult> {
public:
    const RensaCoefResult& result() const { return result_; }

    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) {}
    void nthChainDone(int nthChain, int numErasedPuyo, int coef) { result_.setCoef(nthChain, numErasedPuyo, coef); }

private:
    RensaCoefResult result_;
};
typedef RensaTracker<RensaCoefResult> RensaCoefTracker;

template<>
class RensaTracker<RensaVanishingPositionResult> {
public:
    RensaTracker()
    {
        resetY();
    }

    const RensaVanishingPositionResult& result() const { return result_; }

    void colorPuyoIsVanished(int x, int y, int nthChain)
    {
        if (yAtPrevRensa_[x][y] == 0) {
            result_.setBasePuyo(x, y, nthChain);
        } else {
            result_.setFallingPuyo(x, yAtPrevRensa_[x][y], y, nthChain);
        }
    }

    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void puyoIsDropped(int x, int fromY, int toY) { yAtPrevRensa_[x][toY] = fromY; }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) { resetY(); }

private:
    void resetY() {
        constexpr std::array<int, FieldConstant::MAP_HEIGHT> ALL_ZERO {{}};
        yAtPrevRensa_.fill(ALL_ZERO);
    }

    RensaVanishingPositionResult result_;
    std::array<std::array<int, FieldConstant::MAP_HEIGHT>, FieldConstant::MAP_WIDTH> yAtPrevRensa_;
};
typedef RensaTracker<RensaVanishingPositionResult> RensaVanishingPositionTracker;

// ----------------------------------------------------------------------

// This is the same as RensaChainTracker, however, the result is passed as pointer.
class RensaChainPointerTracker {
public:
    explicit RensaChainPointerTracker(RensaChainTrackResult* trackResult) :
        result_(trackResult)
    {
        // TODO(mayah): Assert trackResult is initialized?
    }

    const RensaChainTrackResult& result() const { return *result_; }

    void colorPuyoIsVanished(int x, int y, int nthChain)
    {
        yTracker_.colorPuyoIsVanished(x, y, nthChain);
        result_->setErasedAt(x, yTracker_.originalY(x, y), nthChain);
    }
    void ojamaPuyoIsVanished(int x, int y, int nthChain)
    {
        yTracker_.ojamaPuyoIsVanished(x, y, nthChain);
        result_->setErasedAt(x, yTracker_.originalY(x, y), nthChain);
    }
    void puyoIsDropped(int x, int fromY, int toY)
    {
        yTracker_.puyoIsDropped(x, fromY, toY);
    }
    void nthChainDone(int nthChain, int numErasedPuyo, int coef)
    {
        yTracker_.nthChainDone(nthChain, numErasedPuyo, coef);
    }

private:
    RensaYPositionTracker yTracker_;
    RensaChainTrackResult* result_;
};

#endif
