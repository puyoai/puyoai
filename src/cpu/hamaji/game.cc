#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bitset>
#include <map>
#include <sstream>

#include <glog/logging.h>

#include "util.h"

Player::Player()
  : score(0), spent_score(0), ojama_cnt(0),
    expected_ojama(0), expected_frame(0) {
  next.resize(6);
}

LF Game::prev_you_can_play_field[2];

void Game::reset() {
  prev_you_can_play_field[0] = LF();
  prev_you_can_play_field[1] = LF();
}

Game::Game()
  : id(0) {
}

Game::Game(const Game& prev_game, const FrameRequest& request) {
  id = request.frameId;
  p[0].f = LF(CoreField(request.myPlayerFrameRequest().field));
  p[0].next = request.myPlayerFrameRequest().kumipuyoSeq;
  p[0].score = request.myPlayerFrameRequest().score;
  p[0].state = request.myPlayerFrameRequest().state;
  p[1].f = LF(CoreField(request.enemyPlayerFrameRequest().field));
  p[1].next = request.enemyPlayerFrameRequest().kumipuyoSeq;
  p[1].score = request.enemyPlayerFrameRequest().score;
  p[1].state = request.enemyPlayerFrameRequest().state;

  decided_field = prev_game.decided_field;

  for (int i = 0; i < 2; i++) {
    for (int x = 1; x <= 6; x++) {
      for (int y = 13; y <= 14; y++) {
        if (p[i].f.Get(x, y-1) == PuyoColor::EMPTY)
          break;
        p[i].f.Set(x, y, prev_game.p[i].f.Get(x, y));
      }
    }
  }

  for (int i = 0; i < 2; i++) {
    if (!p[i].state.grounded) {
      continue;
    }

    if (i == 0) {
      for (int x = 1; x <= 6; x++) {
        for (int y = 13; y <= 14; y++) {
          p[i].f.Set(x, y, prev_game.decided_field.Get(x, y));
        }
      }
    } else {
      // TODO(hamaji): Guess opponent's decision?
    }
  }

  for (int i = 0; i < 2; i++) {
    if (!(p[i].state.decisionRequest || p[i].state.decisionRequestAgain))
      continue;

    // TODO(hamaji): We might not need to check this every frame.
    if (p[i].f.complementOjamasDropped(prev_you_can_play_field[i])) {
      LOG(INFO) << "Ojama guessed for; " << request.toDebugString();
    }

    prev_you_can_play_field[i] = p[i].f;
  }

  p[0].expected_ojama = prev_game.p[0].expected_ojama;
  p[0].expected_frame = prev_game.p[0].expected_frame;
  p[0].spent_score = prev_game.p[0].spent_score;
  p[1].expected_ojama = prev_game.p[1].expected_ojama;
  p[1].expected_frame = prev_game.p[1].expected_frame;
  p[1].spent_score = prev_game.p[1].spent_score;
  if (request.enemyPlayerFrameRequest().state.chainFinished) {
    p[1].expected_ojama = 0;
  }
}

void Game::tick() {
  p[0].expected_frame--;
  p[1].expected_frame--;
}

const string Game::getDebugOutput() const {
  ostringstream oss;
  oss << "ID=" << id << '\n';
  vector<string> lines[2];
  split(p[0].f.GetDebugOutput(), "\n", &lines[0]);
  split(p[1].f.GetDebugOutput(), "\n", &lines[1]);
  for (size_t i = 0; i < lines[0].size(); i++) {
    if (!strncmp(lines[0][i].c_str(), "http", 4)) {
      oss << lines[0][i] << '\n';
      oss << lines[1][i] << '\n';
      break;
    }
    oss << lines[0][i];
    oss << ' ';
    oss << lines[1][i];
    oss << '\n';
  }
  oss << "YOKOKU=" << p[0].next.toString() << "    ";
  oss << "YOKOKU=" << p[1].next.toString() << '\n';
  for (int i = 0; i < 2; i++) {
    const Player& pl = p[i];
    oss << "SC=" << pl.score << " SS=" << pl.spent_score
        << " OC=" << pl.ojama_cnt;
    if (i) {
      oss << " EO=" << pl.expected_ojama << " EF=" << pl.expected_frame;
    }
    oss << '\n';
  }
  oss << p[0].f.url() << '\n';
  oss << p[1].f.url() << '\n';
  return oss.str();
}
