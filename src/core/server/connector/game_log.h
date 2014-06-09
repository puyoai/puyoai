#ifndef CORE_SERVER_CONNECTOR_GAME_LOG_H_
#define CORE_SERVER_CONNECTOR_GAME_LOG_H_

#include <string>
#include <vector>

#include "core/decision.h"
#include "core/key.h"

enum GameResult {
    PLAYING = -1,
    DRAW = 0,
    P1_WIN = 1,
    P2_WIN = 2,
    P1_WIN_WITH_CONNECTION_ERROR = 3,
    P2_WIN_WITH_CONNECTION_ERROR = 4,
};

class ReceivedData {
public:
    std::string original;
    std::string msg;
    std::string mawashi_area;
    int timestamp;
    int frame_id;
    Decision decision;

    void SerializeToString(std::string* output) const;
};

// Information about Jiku-puyo.
struct MovingPosition {
    int x;
    int y;
    int r;
    // TODO(mayah): Why char[2]? Kumipuyo should be used.
    char color[2];
};

class ExecutionData {
public:
    std::vector<Key> keys;
    MovingPosition moving;
    std::vector<int> ojama;
    bool landed;

    ExecutionData() {
        ojama = std::vector<int>(6, 0);
    }

    void SerializeToString(std::string* output) const;
};

class PlayerLog {
public:
    int frame_id;
    int player_id;
    std::vector<ReceivedData> received_data;
    ExecutionData execution_data;
    bool is_human;

    void SerializeToString(std::string* output) const;
};

class GameLog {
public:
    GameResult result;
    std::string error_log;
    std::vector<PlayerLog> log;

    void SerializeToString(std::string* output) const;
};

#endif
