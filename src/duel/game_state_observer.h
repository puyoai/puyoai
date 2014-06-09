#ifndef DUEL_GAME_STATE_OBSERVER_H_
#define DUEL_GAME_STATE_OBSERVER_H_

class GameState;

class GameStateObserver {
public:
    virtual ~GameStateObserver() {}

    virtual void newGameWillStart() {}
    virtual void onUpdate(const GameState&) = 0;
    virtual void gameHasDone() {}
};

#endif
