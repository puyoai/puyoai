#ifndef CPU_PERIA_GAME_H_
#define CPU_PERIA_GAME_H_

#include "base.h"
#include "field.h"

struct Player {
 public:
  void CopyFrom(const Player& player);
  
  int state;
  Field field;
  int score;
  int x;  // x-axis of Jiku puyo
  int y;  // y-axis of Jiku puyo
  int r;  // round number of controlling puyo.
  int ojama;
};

bool operator==(const Player& a, const Player& b);
bool operator!=(const Player& a, const Player& b);

class Game {
 public:
  Game(const std::string& name);
  ~Game();

  // Input game status from the server.
  bool Input(const string& input);

  // Process and output commands.
  string Play();

 private:
  bool CheckDiff(const Player& prev, Player* player);

  std::string name_;
  scoped_ptr<Player> player_;
  scoped_ptr<Player> enemy_;
  bool player_update_;
  bool enemy_update_;
  int id_;  // Time frame
};

#endif  // CPU_PERIA_GAME_H_
