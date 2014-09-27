#ifndef CORE_SERVER_GAME_STATE_OBSERVER_H_
#define CORE_SERVER_GAME_STATE_OBSERVER_H_

#include "core/game_result.h"

class GameState;

class GameStateObserver {
public:
    virtual ~GameStateObserver() {}

    virtual void newGameWillStart() {}
    virtual void onUpdate(const GameState&) = 0;
    virtual void gameHasDone(GameResult) {}
};

#endif
