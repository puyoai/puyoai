#include "core/field/rensa_result.h"

#include <sstream>
#include <iomanip>

using namespace std;

string BasicRensaResult::toString() const
{
    char buf[80];
    sprintf(buf, "chains, score, frames = %d, %d, %d", chains, score, frames);
    return buf;
}

RensaTrackResult& RensaTrackResult::operator=(const RensaTrackResult& result)
{
    for (int x = 0; x < PlainField::MAP_WIDTH; ++x) {
        for (int y = 0; y < PlainField::MAP_HEIGHT; ++y)
            m_erasedAt[x][y] = result.m_erasedAt[x][y];
    }

    return *this;
}

string RensaTrackResult::toString() const
{
    ostringstream ss;
    for (int y = PlainField::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= PlainField::WIDTH; ++x)
            ss << std::setw(3) << static_cast<int>(m_erasedAt[x][y]);
        ss << '\n';
    }

    return ss.str();
}
