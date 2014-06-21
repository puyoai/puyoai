#ifndef DUEL_GAME_H_
#define DUEL_GAME_H_

#include <memory>
#include <string>
#include <vector>

#include "core/decision.h"
#include "core/server/connector/received_data.h"
#include "duel/game_state.h"
#include "duel/game_result.h"

struct Data;
class FieldRealtime;
class PuyoFu;

class Game {
public:
    // Don't take the ownership of DuelServer.
    Game();
    ~Game();

    GameState play(const std::vector<ReceivedData> data[2]);
    GameResult gameResult() const;
    void GetFieldInfo(std::string* player1, std::string* player2) const;

private:
    std::unique_ptr<FieldRealtime> field_[2];
    std::string lastAcceptedMessage_[2];
    Decision latestDecision_[2];
    std::vector<int> ackInfo_[2];
};

#endif  // DUEL_GAME_H_
