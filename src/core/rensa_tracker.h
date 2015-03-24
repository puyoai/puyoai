#ifndef CORE_RENSA_TRACKER_H_
#define CORE_RENSA_TRACKER_H_

#include <glog/logging.h>

#include "core/rensa_result.h"

class RensaNonTracker {
public:
    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) { }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) {}
};

class RensaTracker {
public:
    explicit RensaTracker(RensaTrackResult* trackResult) :
        result_(trackResult)
    {
        // TODO(mayah): Assert trackResult is initialized.
        memcpy(originalY_, s_originalY, sizeof(originalY_));
    }

    void colorPuyoIsVanished(int x, int y, int nthChain) { result_->setErasedAt(x, originalY_[x][y], nthChain); }
    void ojamaPuyoIsVanished(int x, int y, int nthChain) { result_->setErasedAt(x, originalY_[x][y], nthChain); }
    void puyoIsDropped(int x, int fromY, int toY) { originalY_[x][toY] = originalY_[x][fromY]; }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) {}

private:
    static const int s_originalY[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT];

    int originalY_[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT];
    RensaTrackResult* result_;
};

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

class RensaCoefTracker {
public:
    explicit RensaCoefTracker(RensaCoefResult* result) : result_(result) {}

    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) {}
    void nthChainDone(int nthChain, int numErasedPuyo, int coef) { result_->setCoef(nthChain, numErasedPuyo, coef); }

private:
    RensaCoefResult* result_;
};

class RensaVanishingPositionTracker {
public:
    explicit RensaVanishingPositionTracker(RensaVanishingPositionResult* result) : result_(result)
    {
        resetY();
    }

    void colorPuyoIsVanished(int x, int y, int nthChain)
    {
        if (yAtPrevRensa_[x][y] == 0) {
            result_->setBasePuyo(x, y, nthChain);
        } else {
            result_->setFallingPuyo(x, yAtPrevRensa_[x][y], y, nthChain);
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

    RensaVanishingPositionResult* result_;
    std::array<std::array<int, FieldConstant::MAP_HEIGHT>, FieldConstant::MAP_WIDTH> yAtPrevRensa_;
};

#endif
