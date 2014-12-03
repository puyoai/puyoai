#ifndef HAMAJI_RATER_H_
#define HAMAJI_RATER_H_

#include <thread>
#include <mutex>
#include <vector>

#include "base.h"

class RatingStats;
class PuyoCloudManager;

class Rater {
  struct ThreadState {
    ThreadState()
      : rater(NULL), tid(0), game_id(0), turn(0) {}
    Rater* rater;
    int tid;
    int game_id;
    int turn;
  };

public:
  Rater(int eval_threads, int eval_cnt, int base_seed);
  ~Rater();
  void eval(RatingStats* all_stats);
  void GameDone(const int index, const RatingStats& stats);
  static void evalOneGame(int i, int base_seed,
                          ThreadState* state, RatingStats* rating_stats);

private:
  static void* runWorker(void* self);
  void runWorker(int tid);
  int pickGameIndex();

 private:
  std::mutex mu_;

  vector<std::thread> threads_;
  vector<ThreadState> states_;
  vector<RatingStats> rating_stats_vec_;
  int game_index_;
  int finished_games_;
  int eval_cnt_;
  int base_seed_;
#ifdef OS_LINUX
  PuyoCloudManager* puyo_cloud_;
#endif // OS_LINUX
};

#endif  // HAMAJI_RATER_H_
