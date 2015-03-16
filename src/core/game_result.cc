#include "core/game_result.h"

#include <glog/logging.h>

#include <ostream>

GameResult toOppositeResult(GameResult gameResult)
{
    switch (gameResult) {
    case GameResult::PLAYING:
        return GameResult::PLAYING;
    case GameResult::DRAW:
        return GameResult::DRAW;
    case GameResult::P1_WIN:
        return GameResult::P2_WIN;
    case GameResult::P2_WIN:
        return GameResult::P1_WIN;
    case GameResult::P1_WIN_WITH_CONNECTION_ERROR:
        return GameResult::P2_WIN_WITH_CONNECTION_ERROR;
    case GameResult::P2_WIN_WITH_CONNECTION_ERROR:
        return GameResult::P1_WIN_WITH_CONNECTION_ERROR;
    case GameResult::GAME_HAS_STOPPED:
        return GameResult::GAME_HAS_STOPPED;
    default:
        CHECK(false) << "Unknown GameResult: " << static_cast<int>(gameResult);
    }
}
