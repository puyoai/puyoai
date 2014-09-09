#ifndef CORE_PLAYER_H_
#define CORE_PLAYER_H_

#include <sstream>
#include <string>

inline std::string playerText(int playerId)
{
    std::stringstream ss;
    ss << "[P" << (playerId + 1) << "]";
    return ss.str();
}

#endif
