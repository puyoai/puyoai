#include "core/rensa_result.h"

#include <glog/logging.h>

#include <cstddef>
#include <sstream>
#include <iomanip>


using namespace std;

string RensaResult::toString() const
{
    stringstream ss;
    ss << "chains=" << chains
       << " score=" << score
       << " frames=" << frames
       << " quick=" << (quick ? "true" : "false");
    return ss.str();
}

RensaTrackResult::RensaTrackResult() :
    erasedAt_ {}
{
}

RensaTrackResult::RensaTrackResult(const string& s) :
    RensaTrackResult()
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

RensaTrackResult& RensaTrackResult::operator=(const RensaTrackResult& result)
{
    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y)
            erasedAt_[x][y] = result.erasedAt_[x][y];
    }

    return *this;
}

string RensaTrackResult::toString() const
{
    ostringstream ss;
    for (int y = FieldConstant::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= FieldConstant::WIDTH; ++x)
            ss << std::setw(3) << static_cast<int>(erasedAt_[x][y]);
        ss << '\n';
    }

    return ss.str();
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
