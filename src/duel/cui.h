#ifndef DUEL_CUI_H_
#define DUEL_CUI_H_

#include <iostream>
#include <string>
#include <unordered_map>

#include "base/base.h"
#include "duel/game_state_observer.h"

class FieldRealtime;
class GameState;

class Cui : public GameStateObserver {
public:
    Cui() {}
    virtual ~Cui() {}

    void clear();

    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;

private:
    void print(int playerId, const FieldRealtime&, const std::string& message);
    void printField(int playerId, const FieldRealtime&);
    void printNextPuyo(int playerId, const FieldRealtime&);
    void printScore(int playerId, int score);
    void printOjamaPuyo(int playerId, const FieldRealtime&);
    void printMessage(int playerId, const std::string& message);

    void printPuyo(const std::string& location, const std::string& text);
    void printText(const std::string& location, const std::string& text);

    std::unordered_map<std::string, std::string> printPuyoCache_;
    std::unordered_map<std::string, std::string> printTextCache_;
};

#endif  // DUEL_CUI_H_
