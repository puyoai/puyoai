#include "core/game_result.h"

#include <glog/logging.h>

std::string toString(GameResult gameResult)
{
    switch (gameResult) {
    case GameResult::PLAYING:
        return "playing";
    case GameResult::DRAW:
        return "draw";
    case GameResult::P1_WIN:
        return "p1 win";
    case GameResult::P2_WIN:
        return "p2 win";
    case GameResult::P1_WIN_WITH_CONNECTION_ERROR:
        return "p1 win (p2 connection error)";
    case GameResult::P2_WIN_WITH_CONNECTION_ERROR:
        return "p2 win (p1 connection error)";
    case GameResult::GAME_HAS_STOPPED:
        return "stopped";
    }

    CHECK(false) << "Unknown GameResult: " << static_cast<int>(gameResult);
}

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
    }

    CHECK(false) << "Unknown GameResult: " << static_cast<int>(gameResult);
}
