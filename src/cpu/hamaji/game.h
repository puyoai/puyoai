#ifndef HAMAJI_GAME_H_
#define HAMAJI_GAME_H_

#include <string>

#include "core/frame_request.h"
#include "core/kumipuyo_seq.h"
#include "core/state.h"

#include "base.h"
#include "field.h"

class Player {
public:
  Player();

  LF f;
  KumipuyoSeq next;
  UserState state;
  int score;
  int spent_score;
  int ojama_cnt;
  int expected_ojama;
  int expected_frame;
};

class Game {
public:
  Game();
  Game(const Game& prev_game, const FrameRequest&);

  const string getDebugOutput() const;

  void tick();

  int id;
  Player p[2];
  LF decided_field;

  static void reset();
  static LF prev_you_can_play_field[2];
};

#endif  // HAMAJI_GAME_H_
