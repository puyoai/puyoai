#ifndef CORE_PLAYER_H_
#define CORE_PLAYER_H_

#include <sstream>
#include <string>

const int NUM_PLAYERS = 2;

inline std::string playerText(int playerId)
{
    std::stringstream ss;
    ss << "[P" << (playerId + 1) << "]";
    return ss.str();
}

#endif
