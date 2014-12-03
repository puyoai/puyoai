#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base.h"
#include "core.h"
#include "game.h"
#include "rater.h"
#include "ratingstats.h"
#include "solo.h"

DEFINE_bool(solo, false, "");
DEFINE_int32(eval_cnt, 0,
             "Run the game this times and show some stats");
DEFINE_int32(eval_threads, 1, "");
DEFINE_int32(seed, -1, "");
DEFINE_bool(puyo_cloud_worker, false, "work as puyocloud worker");
DEFINE_bool(handle_opponent_grounded, true, "");

int parseState(const string& line) {
  size_t st = line.find("STATE=");
  CHECK_NE(st, string::npos) << line;
  return atoi(&line[st + strlen("STATE=")]);
}

int parseID(const string& line) {
  size_t st = line.find("ID=");
  CHECK_NE(st, string::npos) << line;
  return atoi(&line[st + strlen("ID=")]);
}

int parseEnd(const string& line) {
  size_t st = line.find("END=");
  if (st == string::npos)
    return -2;
  return atoi(&line[st + strlen("END=")]);
}

static Core* g_core;
static bool g_should_decide = true;

bool tick(Game* game) {
  int state = game->state;
#if 0
  bool emergency = false;
#endif
  if ((state & (STATE_YOU_GROUNDED << 1))) {
    int chain_cnt;
    int score;
    int frame_cnt;

    game->p[1].f.SafeDrop();
    LF f(game->p[1].f);
    f.Simulate(&chain_cnt, &score, &frame_cnt);
    if (score) {
      score += game->p[1].score - game->p[1].spent_score;
      int ojama_cnt = score / 70;
      game->p[1].spent_score += ojama_cnt * 70;
      game->p[1].expected_ojama += ojama_cnt;
      game->p[1].expected_frame = frame_cnt;
#if 0
      emergency = FLAGS_handle_opponent_grounded;
#endif
      LOG(INFO) << "Opponent fired! ojama=" << game->p[1].expected_ojama
                << " frames=" << game->p[1].expected_frame
                << "\n" << game->p[1].f.GetDebugOutput();
    }
  }
  if ((state & STATE_YOU_GROUNDED)) {
    g_should_decide = true;
  }
#if 0
  // TODO(hamaji): Emergency handling was removed.
  //if ((g_should_decide && (state & STATE_YOU_CAN_PLAY)) || emergency) {
#endif
  if (g_should_decide && (state & STATE_YOU_CAN_PLAY)) {
    g_should_decide = false;
    Decision decision = g_core->decide(game);
    char buf[1024];
    int i = sprintf(buf, "ID=%d X=%d R=%d MSG=%-40s MA=",
                    game->id, decision.x, decision.r, g_core->msg().c_str());
    g_core->clear_msg();
    for (int y = 13; y <= 14; y++) {
      for (int x = 1; x <= 6; x++) {
        char c = game->p[0].f.Get(x, y) + '0';
        buf[i++] = c;
      }
    }
    buf[i] = 0;
    if (decision.isValid()) {
      game->decided_field = game->p[0].f;
      game->decided_field.PutDecision(decision,
                                      game->p[0].next[0], game->p[0].next[1]);
    }
    LOG(INFO) << "<= " << buf
              << " (cnt=" << game->p[0].f.countPuyo() << ")";
    puts(buf);
    fflush(stdout);
    return true;
  }
  return false;
}

void letsPuyoShobu() {
  char line_buf[1024];
  line_buf[1023] = '\0';

  CHECK(fgets(line_buf, 1023, stdin));
  puts(line_buf);
  fflush(stdout);

  LOG(INFO) << "Start!";

  g_should_decide = true;
  auto_ptr<Game> prev_game(new Game());
  g_core = new Core(false);

  for (; fgets(line_buf, 1023, stdin); prev_game->tick()) {
    google::FlushLogFiles(google::INFO);

    const string& line = line_buf;
    CHECK_NE(line.size(), 1023UL) << line;

    VLOG(1) << line;
    int winner = parseEnd(line);
    if (winner >= -1) {
      Game::reset();
      LOG(INFO) << line;
      switch (winner) {
      case -1:
        LOG(INFO) << "=== YOU /(^o^)\\ LOSE ===";
        break;
      case 0:
        LOG(INFO) << "=== DRAW -(-_-)- GAME ===";
        break;
      case 1:
        LOG(INFO) << "=== YOU \\(^o^)/ WIN ===";
        break;
      }
      g_should_decide = true;
      prev_game.reset(new Game());
      continue;
    }

    int state = parseState(line);
    if (!state) {
      int id = parseID(line);
      printf("ID=%d\n", id);
      fflush(stdout);
      continue;
    } else {
      VLOG(1) << "=> " << line;
    }

    auto_ptr<Game> game(new Game(*prev_game, line));

    CHECK_EQ(state, game->state);

    if (!tick(game.get())) {
      int id = parseID(line);
      printf("ID=%d\n", id);
      fflush(stdout);
    }

    prev_game = game;
  }
}

void tokotonPuyoPuyo(int seed) {
  SoloGame solo(seed, true);
  solo.run();
}

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

  if (FLAGS_seed < 0) {
    srand(time(NULL));
    FLAGS_seed = rand();
  }
  LOG(INFO) << "seed=" << FLAGS_seed;

  if (FLAGS_eval_cnt > 0) {
    Rater rater(FLAGS_eval_threads, FLAGS_eval_cnt, FLAGS_seed);
    RatingStats all_stats;
    rater.eval(&all_stats);
    all_stats.Print();
  } else if (FLAGS_solo) {
    tokotonPuyoPuyo(FLAGS_seed);
  } else {
    letsPuyoShobu();
  }
}
