#ifndef HAMAJI_GAME_H_
#define HAMAJI_GAME_H_

#include <string>

#include "../../core/state.h"

#include "base.h"
#include "field.h"

class Player {
public:
  Player();

  LF f;
  string next;
  int score;
  int spent_score;
  int ojama_cnt;
  int expected_ojama;
  int expected_frame;
};

class Game {
public:
  Game();
  Game(const Game& prev_game, const string& line);

  const string getDebugOutput() const;

  void tick();

  Player p[2];
  int state;
  int id;
  LF decided_field;

  static void reset();
  static LF prev_you_can_play_field[2];
};

#endif  // HAMAJI_GAME_H_
