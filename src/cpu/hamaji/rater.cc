#include "rater.h"

#include <stdio.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <chrono>
#include <thread>

#include "field.h"
#include "game.h"
#include "ratingstats.h"
#include "solo.h"

// #define GOOGLE3
#ifdef GOOGLE3
#include "puyocloud.h"
#endif  // GOOGLE3

DEFINE_int32(eval_turns, 40, "");

DEFINE_bool(puyo_cloud, false, "Run on borg or not");
DEFINE_bool(optimistic_chain_count, true, "Be optimistic");

DEFINE_bool(show_progress, true, "");

Rater::Rater(int eval_threads, int eval_cnt, int base_seed)
    : threads_(eval_threads),
      states_(eval_threads),
      game_index_(0),
      finished_games_(0),
      eval_cnt_(eval_cnt),
      base_seed_(base_seed) {
#ifdef GOOGLE3
  puyo_cloud_ = NULL;
  if (FLAGS_puyo_cloud) {
    puyo_cloud_ = new PuyoCloudManager();
    puyo_cloud_->SetNumRequests(eval_cnt);
  }
#endif  // GOOGLE3
}

Rater::~Rater() {
#ifdef GOOGLE3
  if (puyo_cloud_ != NULL) {
    delete puyo_cloud_;
  }
#endif  // GOOGLE3
}
void Rater::eval(RatingStats* all_stats) {
  for (size_t i = 0; i < threads_.size(); i++) {
    states_[i].tid = i;
    states_[i].rater = this;
    threads_[i] = std::thread([i, this]() {
        Rater::runWorker((void*)&states_[i]);
    });
  }

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::lock_guard<std::mutex> lock(mu_);

    if (FLAGS_show_progress) {
      fprintf(stderr, "\r");
      int sum_turns = 0;
      for (size_t i = 0; i < states_.size(); i++) {
        const ThreadState& s = states_[i];
        sum_turns += s.turn;
      }
      fprintf(stderr, "%d/%d %d    ",
              finished_games_, eval_cnt_, sum_turns);
    }

    if (finished_games_ == eval_cnt_)
      break;
  }

  for (size_t i = 0; i < threads_.size(); i++) {
    if (threads_[i].joinable())
      threads_[i].join();
  }
#ifdef GOOGLE3
  if (FLAGS_puyo_cloud) {
    puyo_cloud_->Wait();
  }
#endif // GOOGLE3
  std::lock_guard<std::mutex> lock(mu_);
  for (size_t i = 0; i < rating_stats_vec_.size(); ++i) {
    all_stats->merge(rating_stats_vec_[i]);
  }
}

void* Rater::runWorker(void* self) {
  ThreadState* st = (ThreadState*)self;
  st->rater->runWorker(st->tid);
  return NULL;
}

void Rater::runWorker(int tid) {
  while (true) {
    int i = pickGameIndex();
    if (i == -1)
      break;

    if (!FLAGS_puyo_cloud) {
      RatingStats rating_stats;
      evalOneGame(i, base_seed_ + i, &(states_[tid]), &rating_stats);
      GameDone(i, rating_stats);
    } else {
#ifdef GOOGLE3
      puyo_cloud_->SendToPuyoCloud(i, this);
#else  // GOOGLE3
      LOG(FATAL) << "PuyoCloud is supported only in google3.";
#endif  // GOOGLE3
    }
  }
}

void Rater::GameDone(const int /* index */, const RatingStats& stats) {
  std::lock_guard<std::mutex> lock(mu_);
  // LOG(INFO) << rating_stats_vec_.size() << " done.";
  rating_stats_vec_.push_back(stats);
  finished_games_++;
}

int Rater::pickGameIndex() {
  std::lock_guard<std::mutex> lock(mu_);
  if (game_index_ == eval_cnt_)
    return -1;
  return game_index_++;
}

void Rater::evalOneGame(int i,
                        int seed,
                        ThreadState* state,
                        RatingStats* rating_stats) {
  rating_stats->total_count++;
  SoloGame game(seed, true);
  int maxScore = 0;
  string maxURL;
  int prev_chigiri_frames = 0;
  for (int t = 0; t < FLAGS_eval_turns; t++) {
    clock_t start = clock();
    int r = game.step();
    float elapsedMsec = (float)(clock() - start) / CLOCKS_PER_SEC * 1000;

    if (state != NULL) {
      state->game_id = i;
      state->turn = t;
    }

    rating_stats->add_elapsed_msec(elapsedMsec);
    rating_stats->add_chigiri_frames(
      game.chigiri_frames - prev_chigiri_frames);
    prev_chigiri_frames = game.chigiri_frames;
    game.pickNext();
    if (r == -1) {
      rating_stats->add_invalid_stat(i, t);
      if (FLAGS_show_progress)
        puts("INVALID");
      break;
    }
    if (r == -2) {
      rating_stats->add_dead_stat(i, t);
      puts("DEAD");
      break;
    }

    LF nf(game.game->p[0].f);
    int score = game.cpu.best_score();
    int num_chains = game.cpu.best_chain();

    for (int c = 2; c <= num_chains; c++) {
      rating_stats->add_chain_stats(c, i, t);
    }
    if (maxScore < score) {
      maxURL = nf.url();
      maxScore = score;
    }
  }

  rating_stats->add_max_score(maxScore, maxURL);
}
