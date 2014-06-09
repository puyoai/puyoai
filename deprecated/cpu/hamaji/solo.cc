#include "solo.h"

#include <stdio.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core.h"
#include "game.h"

DEFINE_bool(gekikara, false, "");
DEFINE_string(initial_state, "",
              "If specified, load the file as the initial state");
DEFINE_string(puyo_sequence, "",
              "If specified, use this sequence before the random puyos");

DEFINE_int32(tokopuyo_turns, 0, "");

SoloGame::SoloGame(int seed, bool evalmode)
  : cpu(true),
    mt(seed),
    evalmode_(evalmode),
    chigiri_frames(0),
    puyo_sequence_index_(0) {
  /*
  if (FLAGS_gekikara) {
    for (int y = 10; y < 15; y++) {
      for (int x = 1; x <= 6; x++) {
        field.Set(x, y, OJAMA);
      }
    }
  } else if (!FLAGS_initial_state.empty()) {
    FILE* fp = fopen(FLAGS_initial_state.c_str(), "rb");
    CHECK(fp) << FLAGS_initial_state;
    vector<string> lines;
    char buf[10];
    memset(buf, 0, sizeof(buf));
    while (fgets(buf, 9, fp)) {
      CHECK_GE(8, strlen(buf));
      if (strlen(buf) == 0)
        break;
      lines.push_back(buf);
    }
    fclose(fp);

    for (size_t i = 0; i < lines.size(); i++) {
      const string& l = lines[i];
      for (size_t x = 0; x < 6; x++) {
        if (x >= l.size())
          break;
        char c = l[x];
        switch (c) {
        case '@':
          c = OJAMA;
          break;
        case 'A':
          c = RED;
          break;
        case 'B':
          c = BLUE;
          break;
        case 'C':
          c = YELLOW;
          break;
        case 'D':
          c = GREEN;
          break;
        default:
          c = EMPTY;
        }
        field.Set(x+1, 15-lines.size()+i, c);
      }
    }

    // if (!evalmode_)
    printf("%s\n", field.GetDebugOutput().c_str());
  }
  */

  game = new Game();

  next.resize(6);
  for (int i = 0; i < 6; i++) {
    next[i] = pickNextPuyo();
  }
}

SoloGame::~SoloGame() {
  delete game;
}

int SoloGame::pickNextPuyo() {
  if (puyo_sequence_index_ < static_cast<int>(FLAGS_puyo_sequence.size())) {
    return FLAGS_puyo_sequence[puyo_sequence_index_++] - '0';
  }
  return mt.genrand_int32() % 4 + 4;
}

void SoloGame::pickNext() {
  next[0] = next[2];
  next[1] = next[3];
  next[2] = next[4];
  next[3] = next[5];
  next[4] = pickNextPuyo();
  next[5] = pickNextPuyo();
}

int SoloGame::run() {
  for (int turn = 0;
       !FLAGS_tokopuyo_turns || turn < FLAGS_tokopuyo_turns;
       turn++) {
    game->p[0].next = next;
    Decision d = cpu.decide(game);
    printf("turn=%d x=%d r=%d msg=%s\n", turn, d.x, d.r, cpu.msg().c_str());
    if (!d.IsValid()) {
      printf("Broken decision\n");
      return 0;
    }
    int score = game->p[0].f.PutDecision(d, next[0], next[1], &chigiri_frames);
    printf("%s\n", game->p[0].f.GetDebugOutput(game->p[0].next).c_str());
    /*
    F::showNext(next);
    field.show();
    field.removeHighlight();
    field.showURL();
    */
    pickNext();

    if (score < 0) {
      printf("Dead\n");
      return 0;
    }
    if (score) {
      printf("score=%d\n", score);
    }

    fflush(stdout);
  }
  return 0;
}

// Returns the score earned in this turn.
// -1 for invalid CPU response
// -2 if died
int SoloGame::step() {
  game->p[0].next = next;
  Decision d = cpu.decide(game);
  if (!d.IsValid()) {
    return -1;
  }

  int score = game->p[0].f.PutDecision(d, next[0], next[1], &chigiri_frames);
  if (score < 0)
    return -2;

  return score;
}

/*
int SoloGame::run2()
{
  int best_x, best_y;
  Plan::evaltaichiinternal(field, &best_x, &best_y, true);
  vector<IgnitionPoint> ignPoints;
  field.getIgnitionPoints(&ignPoints);

  // TODO
  return 0;
}
*/
