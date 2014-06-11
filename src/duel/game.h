#ifndef DUEL_GAME_H_
#define DUEL_GAME_H_

#include <memory>
#include <string>
#include <vector>

// TODO(mayah): Depending server connector looks wrong.
#include "core/decision.h"
#include "duel/game_state.h"
#include "duel/game_result.h"

struct Data;
class DuelServer;
class FieldRealtime;
class PuyoFu;
class UserInput;

class Game {
public:
    // Don't take the ownership of DuelServer and/or UserInput.
    // UserInput can be nullptr.
    Game(DuelServer*, UserInput*);
    ~Game();

    void Play(const std::vector<PlayerLog>& all_data);
    GameResult::Result GetWinner(int* scores) const;
    void GetFieldInfo(std::string* player1, std::string* player2) const;

private:
    DuelServer* duelServer_;
    UserInput* userInput_;

    std::unique_ptr<FieldRealtime> field[2];
    std::string last_accepted_messages_[2];
    Decision latest_decision_[2];
    std::vector<int> ack_info_[2];
};

#endif  // DUEL_GAME_H_
