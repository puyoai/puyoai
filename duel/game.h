#ifndef DUEL_GAME_H_
#define DUEL_GAME_H_

#include <string>
#include <vector>

#include "core/ctrl.h"
#include "core/decision.h"
#include "duel/game_log.h"
#include "duel/ojama_controller.h"

class Data;
class FieldRealtime;

class Game {
 private:
  FieldRealtime* field[2];
  Decision latest_decision_[2];
  OjamaController ojama_ctrl_[2];
  std::vector<int> ack_info_[2];

 public:
  Game();
  ~Game();
  void Play(
      const std::vector<PlayerLog>& all_data,
      GameLog* log);
  GameResult GetWinner(int* scores) const;
  void GetFieldInfo(std::string* player1, std::string* player2) const;
};

#endif  // DUEL_GAME_H_
