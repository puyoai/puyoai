#include "core/rensa_track_result.h"

#include <glog/logging.h>

#include <cstddef>
#include <sstream>
#include <iomanip>

#include "core/score.h"

using namespace std;

RensaChainTrackResult::RensaChainTrackResult() :
    erasedAt_ {}
{
}

RensaChainTrackResult::RensaChainTrackResult(const string& s) :
    RensaChainTrackResult()
{
    CHECK(s.size() % 6 == 0) << s.size();

    int height = static_cast<int>(s.size() / 6);
    for (size_t i = 0; i < s.size(); ++i) {
        size_t x = (i % 6) + 1;
        size_t y = height - (i / 6);
        if (s[i] == ' ' || s[i] == '.')
            continue;

        if ('0' <= s[i] && s[i] <= '9')
            erasedAt_[x][y] = s[i] - '0';
        else if ('A' <= s[i] && s[i] <= 'F')
            erasedAt_[x][y] = s[i] - 'A' + 10;
        else if ('a' <= s[i] && s[i] <= 'f')
            erasedAt_[x][y] = s[i] - 'a' + 10;
        else
            CHECK(false) << s[i];
    }
}

RensaChainTrackResult& RensaChainTrackResult::operator=(const RensaChainTrackResult& result)
{
    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y)
            erasedAt_[x][y] = result.erasedAt_[x][y];
    }

    return *this;
}

string RensaChainTrackResult::toString() const
{
    ostringstream ss;
    for (int y = FieldConstant::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= FieldConstant::WIDTH; ++x)
            ss << std::setw(3) << static_cast<int>(erasedAt_[x][y]);
        ss << '\n';
    }

    return ss.str();
}

int RensaCoefResult::score(int additionalChain) const
{
    int rensaCoef[20] {};
    int rensaNumErased[20] {};
    for (int i = 1; i <= additionalChain; ++i) {
        rensaCoef[i] = chainBonus(i);
        if (rensaCoef[i] == 0)
            rensaCoef[i] = 1;
        rensaNumErased[i] = 4;
    }

    for (int i = additionalChain + 1, j = 1; i <= 19 && numErased(j) > 0; ++i, ++j) {
        rensaCoef[i] = calculateRensaBonusCoef(chainBonus(i), longBonusCoef(j), colorBonusCoef(j));
        rensaNumErased[i] = numErased(j);
    }

    int sum = 0;
    for (int i = 1; i <= 19 && rensaNumErased[i] > 0; ++i) {
        sum += rensaCoef[i] * rensaNumErased[i] * 10;
    }

    return sum;
}

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
