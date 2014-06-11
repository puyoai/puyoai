#ifndef DUEL_GAME_RESULT_H_
#define DUEL_GAME_RESULT_H_

struct GameResult {
    enum Result {
        PLAYING = -1,
        DRAW = 0,
        P1_WIN = 1,
        P2_WIN = 2,
        P1_WIN_WITH_CONNECTION_ERROR = 3,
        P2_WIN_WITH_CONNECTION_ERROR = 4,
    };

    Result result;
    std::string errorLog;
};

#endif
