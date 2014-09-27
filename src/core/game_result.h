#ifndef CORE_GAME_RESULT_H_
#define CORE_GAME_RESULT_H_

enum class GameResult {
    PLAYING,
    DRAW,
    P1_WIN,
    P2_WIN,
    P1_WIN_WITH_CONNECTION_ERROR,
    P2_WIN_WITH_CONNECTION_ERROR,
    GAME_HAS_STOPPED,
};

GameResult toOppositeResult(GameResult);

inline GameResult fromRequestEnd(int end)
{
    if (end == 1) {
        return GameResult::P1_WIN;
    } else if (end == 0) {
        return GameResult::DRAW;
    } else if (end == -1) {
        return GameResult::P2_WIN;
    }

    return GameResult::PLAYING;
}


#endif
