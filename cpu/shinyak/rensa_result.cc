#include "rensa_result.h"

#include <sstream>
#include <iomanip>

using namespace std;

string BasicRensaInfo::toString() const
{
    char buf[80];
    sprintf(buf, "chains, score, frames = %d, %d, %d", chains, score, frames);
    return buf;
}

TrackResult& TrackResult::operator=(const TrackResult& result)
{
    for (int x = 0; x < Field::MAP_WIDTH; ++x) {
        for (int y = 0; y < Field::MAP_HEIGHT; ++y)
            m_erasedAt[x][y] = result.m_erasedAt[x][y];
    }
    
    return *this;
}

string TrackResult::toString() const
{
    ostringstream ss;
    for (int y = Field::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= Field::WIDTH; ++x)
            ss << std::setw(3) << static_cast<int>(m_erasedAt[x][y]);
        ss << '\n';
    }
    
    return ss.str();
}
