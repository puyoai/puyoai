#include "core/field/rensa_result.h"

#include <sstream>
#include <iomanip>

#include <glog/logging.h>

using namespace std;

string RensaResult::toString() const
{
    stringstream ss;
    ss << "chains=" << chains
       << " score=" << score
       << " frames=" << frames;
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

    int height = static_cast<int>(s.size() % 6) + 1;
    for (size_t i = 0; i < s.size(); ++i) {
        size_t x = i % 6;
        size_t y = height - (i / 6);
        if ('0' <= s[i] && s[i] <= '9')
            erasedAt_[x][y] = s[i] - '0';
        else if ('A' <= s[i] && s[i] <= 'F')
            erasedAt_[x][y] = s[i] - 'A';
        else if ('a' <= s[i] && s[i] <= 'f')
            erasedAt_[x][y] = s[i] - 'a';
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
