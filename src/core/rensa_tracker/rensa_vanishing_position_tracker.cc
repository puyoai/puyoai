#include "core/rensa_tracker/rensa_vanishing_position_tracker.h"

void RensaVanishingPositionResult::setFallingPuyo(int x, int yBeforeFall, int yAfterFall, int nthChain) {
    maybeResize(nthChain);
    fallingPuyosErasedAt_[nthChain - 1].push_back(Position(x, yBeforeFall));
    yOfFalledPuyosErasedAt_[nthChain - 1].push_back(yAfterFall);
}

void RensaVanishingPositionResult::setBasePuyo(int x, int y, int nthChain) {
    maybeResize(nthChain);
    basePuyosErasedAt_[nthChain - 1].push_back(Position(x, y));
}

void RensaVanishingPositionResult::maybeResize(int nthChain) {
    if ((int) basePuyosErasedAt_.size() < nthChain) {
        basePuyosErasedAt_.resize(nthChain);
        fallingPuyosErasedAt_.resize(nthChain);
        yOfFalledPuyosErasedAt_.resize(nthChain);
    }
}

std::array<float, 2> RensaVanishingPositionResult::getWeightedCenterAfterFall(int nthChain) const {
    std::array<float, 2> weightedCenter = {{0, 0}};

    for (Position pos : basePuyosErasedAt_[nthChain - 1]) {
        weightedCenter[0] += pos.x;
        weightedCenter[1] += pos.y;
    }

    for (Position pos : fallingPuyosErasedAt_[nthChain - 1]) {
        weightedCenter[0] += pos.x;
    }

    for (int y : yOfFalledPuyosErasedAt_[nthChain - 1]) {
        weightedCenter[1] += y;
    }

    auto puyosCount = basePuyosErasedAt_[nthChain - 1].size() + fallingPuyosErasedAt_[nthChain].size();
    if (puyosCount != 0) {
        weightedCenter[0] /= puyosCount;
        weightedCenter[1] /= puyosCount;
    }

    return weightedCenter;
}
