#include "core/field/rensa_result.h"

#include <sstream>
#include <iomanip>

using namespace std;

string RensaResult::toString() const
{
    stringstream ss;
    ss << "chains=" << chains
       << " score=" << score
       << " frames=" << frames;
    return ss.str();
}

RensaTrackResult& RensaTrackResult::operator=(const RensaTrackResult& result)
{
    for (int x = 0; x < PlainField::MAP_WIDTH; ++x) {
        for (int y = 0; y < PlainField::MAP_HEIGHT; ++y)
            erasedAt_[x][y] = result.erasedAt_[x][y];
    }

    return *this;
}

string RensaTrackResult::toString() const
{
    ostringstream ss;
    for (int y = PlainField::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= PlainField::WIDTH; ++x)
            ss << std::setw(3) << static_cast<int>(erasedAt_[x][y]);
        ss << '\n';
    }

    return ss.str();
}
