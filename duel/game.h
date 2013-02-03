#ifndef __GAME_H__
#define __GAME_H__

#include <string>
#include <vector>

#include "ctrl.h"
#include "decision.h"
#include "game_log.h"
#include "ojama_controller.h"

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

#endif  // __GAME_H__
