#include <string>

#include <random>

#include "base.h"
#include "core.h"

class Game;

class SoloGame {
 public:
  SoloGame(int game_index, bool evalmode);
  ~SoloGame();
  int pickNextPuyo();
  void pickNext();
  int run();
  //int run2();
  int step();

  Game* game;
  Core cpu;
  string next;
  std::mt19937 mt;
  bool evalmode_;
  int chigiri_frames;
  int puyo_sequence_index_;
};
