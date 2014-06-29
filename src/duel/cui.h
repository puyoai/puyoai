#ifndef DUEL_CUI_H_
#define DUEL_CUI_H_

#include <iostream>
#include <string>

#include "base/base.h"
#include "duel/game_state_observer.h"

class FieldRealtime;
class GameState;

class Cui : public GameStateObserver {
public:
    Cui() {}
    virtual ~Cui() {}

    void clear();

    virtual void newGameWillStart() OVERRIDE;
    virtual void onUpdate(const GameState&) OVERRIDE;

private:
    void Print(int player_id, const FieldRealtime&, const std::string& debug_message);
    void PrintField(int player_id, const FieldRealtime&);
    void PrintNextPuyo(int player_id, const FieldRealtime&);
    void PrintOjamaPuyo(int player_id, const FieldRealtime&);
    void PrintDebugMessage(int player_id, const std::string& debug_message);
};

#endif  // DUEL_CUI_H_
