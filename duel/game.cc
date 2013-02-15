#include "game.h"

#include <cstdlib>
#include <iostream>
#include <glog/logging.h>
#include <sstream>
#include <string>
#include <vector>

#include "core/ctrl.h"
#include "core/data.h"
#include "core/decision.h"
#include "core/state.h"
#include "duel/field_realtime.h"
#include "duel/game_log.h"
#include "duel/gui.h"
#include "duel/ojama_controller.h"
#include "duel/puyofu.h"

using namespace std;

static Gui* g_gui;

Game::Game() {
  if (!g_gui)
    g_gui = Gui::Create();

  puyo_fu_.reset(new PuyoFu());

  std::string sequence(256, EMPTY);
  char colors[] = {RED, BLUE, GREEN, YELLOW};
  for (int i = 0; i < 256; i++) {
    sequence[i] = colors[std::rand() % 4] + '0';
  }
  const char* puyo_seq = getenv("PUYO_SEQ");
  if (puyo_seq) {
    for (int i = 0; i < 256 && puyo_seq[i]; i++) {
      char c = puyo_seq[i];
      if (c < '4' || c > '7') {
        cerr << "Broken PUYO_SEQ=" << puyo_seq << endl;
        abort();
      }
      sequence[i] = puyo_seq[i];
    }
  }
  LOG(INFO) << "Puyo sequence=" << sequence;

  for (int i = 0; i < 2; i++) {
    ojama_ctrl_[i].SetOpponent(&ojama_ctrl_[1 - i]);
    field[i] = new FieldRealtime(i, sequence, &ojama_ctrl_[i]);
    latest_decision_[i] = Decision::NoInputDecision();
  }
}

Game::~Game() {
  if (getenv("PUYO_PUYOFU")) {
    FILE* fp;
    fp = fopen("/tmp/puyoai_1p.txt", "a");
    puyo_fu_->emitFieldTransitionLog(fp, 0);
    fprintf(fp, "=== end ===\n");
    fclose(fp);
    fp = fopen("/tmp/puyoai_2p.txt", "a");
    puyo_fu_->emitFieldTransitionLog(fp, 1);
    fprintf(fp, "=== end ===\n");
    fclose(fp);
  }

  for (int i = 0; i < 2; i++) {
    delete field[i];
  }
}

/**
 * Updates decision when an applicable one is found.
 * Returns:
 *   if there is an accepted decision:
 *     its index in the given data array.
 *   else:
 *     -1
 */
int UpdateDecision(
    const PlayerLog& data,
    FieldRealtime* field,
    Decision* decision) {
  // Try all commands from the newest one.
  // If we find a command we can use, we'll ignore older ones.
  for (unsigned int i = data.received_data.size(); i > 0; ) {
    i--;

    // We don't need ACK/NACK for ID only messages.
    if (data.received_data[i].decision.x == 0 &&
        data.received_data[i].decision.r == 0) {
      continue;
    }

    Decision d = Decision(
        data.received_data[i].decision.x,
        data.received_data[i].decision.r);

    if (d.IsValid()) {
      Key key = field->GetKey(d);
      if (key != KEY_NONE) {
        *decision = d;
        return i;
      }
    } else if (d.x == -1 && d.r == -1) {
      *decision = d;
      return i;
    }
  }
  return -1;
}

void Game::Play(
    const vector<PlayerLog>& data,
    GameLog* log) {
  for (int i = 0; i < data.size(); i++) {
    FieldRealtime* me = field[i];
    string accepted_message;
    Key key;

    if (data[i].is_human) {
      key = g_gui->GetKey();
    } else {
      int accepted_index = UpdateDecision(
          data[i],
          field[i],
          &latest_decision_[i]);

      // Take care of ack_info.
      ack_info_[i] = vector<int>(data[i].received_data.size(), 0);
      for (int j = 0; j < data[i].received_data.size(); j++) {
        const ReceivedData& d = data[i].received_data[j];
        // This case does not require ack.
        if (d.decision.x == 0 &&
            d.decision.r == 0) {
          continue;
        }
        if (j == accepted_index) {
          ack_info_[i][j] = d.frame_id;
        } else {
          ack_info_[i][j] = -d.frame_id;
        }
      }

      if (accepted_index != -1) {
        accepted_message =
          data[i].received_data[accepted_index].msg;
      }

      key = me->GetKey(latest_decision_[i]);
    }

    PlayerLog player_log = data[i];
    me->Play(key, &player_log);

    {
      int moving_x;
      int moving_y;
      int moving_r;
      char c1, c2;
      {
        int x2, y2;
        me->GetCurrentPuyo(&moving_x, &moving_y, &c1,
                           &x2, &y2, &c2, &moving_r);
      }
      player_log.execution_data.keys.push_back(key);
      player_log.execution_data.moving.x = moving_x;
      player_log.execution_data.moving.y = moving_y;
      player_log.execution_data.moving.r = moving_r;
      player_log.execution_data.moving.color[0] = c1 + '0';
      player_log.execution_data.moving.color[1] = c2 + '0';
      player_log.execution_data.landed =
        me->GetStateInfo() & STATE_YOU_GROUNDED;
      log->log.push_back(player_log);
    }

    // Clear current key input if the move is done.
    if ((me->GetStateInfo() & STATE_YOU_GROUNDED)) {
      latest_decision_[i] = Decision::NoInputDecision();
    }

    if ((me->GetStateInfo() & ~STATE_YOU_CAN_PLAY) != 0) {
      puyo_fu_->setField(me->player_id(),
                         *me,
                         me->GetStateInfo(),
                         0  /* TODO(hamaji): send timestamp? */);
    }

    g_gui->Draw(*me, accepted_message);
    if (!me->IsDead()) {
      me->Print(accepted_message);
    }
  }
  g_gui->Flip();
}

GameResult Game::GetWinner(int* scores) const {
  scores[0] = field[0]->GetScore();
  scores[1] = field[1]->GetScore();
  bool p1_dead = field[0]->IsDead();
  bool p2_dead = field[1]->IsDead();
  if (!p1_dead && !p2_dead) {
    return PLAYING;
  }
  if (p1_dead && p2_dead) {
    return DRAW;
  }
  if (p1_dead) {
    return P2_WIN;
  }
  if (p2_dead) {
    return P1_WIN;
  }
  return PLAYING;
}

std::string FormatAckInfo(vector<int> ack_info) {
  stringstream ss;
  int accepted_id = -1;
  bool has_nack = false;
  for (int i = 0; i < ack_info.size(); i++) {
    if (ack_info[i] > 0) {
      accepted_id = ack_info[i];
    } else if (ack_info[i] < 0) {
      if (has_nack) {
        ss << ",";
      }
      ss << -ack_info[i];
      has_nack = true;
    }
  }
  stringstream ret;
  if (accepted_id > 0) {
    ret << "ACK=" << accepted_id << " ";
  }
  if (has_nack) {
    ret << "NACK=" << ss.str();
  }
  return ret.str();
}

void Game::GetFieldInfo(std::string* player1, std::string* player2) const {
  std::string f0 = field[0]->GetFieldInfo();
  std::string f1 = field[1]->GetFieldInfo();
  std::string y0 = field[0]->GetYokokuInfo();
  std::string y1 = field[1]->GetYokokuInfo();
  int state0 = field[0]->GetStateInfo();
  int state1 = field[1]->GetStateInfo();
  int score0 = field[0]->GetScore();
  int score1 = field[1]->GetScore();
  int ojama0 = ojama_ctrl_[0].GetYokokuOjama();
  int ojama1 = ojama_ctrl_[1].GetYokokuOjama();
  std::string ack0 = FormatAckInfo(ack_info_[0]);
  std::string ack1 = FormatAckInfo(ack_info_[1]);

  int pos_x_0, pos_y_0, r0;
  int pos_x_1, pos_y_1, r1;
  int dummy1, dummy2;
  char c0, c1, dummy3;
  field[0]->GetCurrentPuyo(&pos_x_0, &pos_y_0, &c0,
                           &dummy1, &dummy2, &dummy3, &r0);
  field[1]->GetCurrentPuyo(&pos_x_1, &pos_y_1, &c1,
                           &dummy1, &dummy2, &dummy3, &r1);

  string win0, win1;
  int scores[2];
  int winner = GetWinner(scores);
  switch (winner) {
  case -1:
    break;
  case 0:
    win0 = "END=1 ";
    win1 = "END=-1 ";
    break;
  case 1:
    win0 = "END=-1 ";
    win1 = "END=1 ";
    break;
  case 2:
    win0 = win1 = "END=0 ";
    break;
  default:
    abort();
  }

  {
    std::stringstream ss;
    ss << "STATE=" << (state0 + (state1 << 1)) << " "
       << "YF=" << f0 << " "
       << "OF=" << f1 << " "
       << "YP=" << y0 << " "
       << "OP=" << y1 << " "
       << "YX=" << pos_x_0 << " "
       << "YY=" << pos_y_0 << " "
       << "YR=" << r0 << " "
       << "OX=" << pos_x_1 << " "
       << "OY=" << pos_y_1 << " "
       << "OR=" << r1 << " "
       << "YO=" << ojama0 << " "
       << "OO=" << ojama1 << " "
       << "YS=" << score0 << " "
       << "OS=" << score1 << " "
       << win0
       << ack0;
    *player1 = ss.str();
  }

  {
    std::stringstream ss;
    ss << "STATE=" << (state1 + (state0 << 1)) << " "
       << "YF=" << f1 << " "
       << "OF=" << f0 << " "
       << "YP=" << y1 << " "
       << "OP=" << y0 << " "
       << "YX=" << pos_x_1 << " "
       << "YY=" << pos_y_1 << " "
       << "YR=" << r1 << " "
       << "OX=" << pos_x_0 << " "
       << "OY=" << pos_y_0 << " "
       << "OR=" << r0 << " "
       << "YO=" << ojama1 << " "
       << "OO=" << ojama0 << " "
       << "YS=" << score1 << " "
       << "OS=" << score0 << " "
       << win1
       << ack1;
    *player2 = ss.str();
  }

  if (state0 & STATE_YOU_CAN_PLAY) {
    LOG(INFO) << "Player 0 can play.";
  }
  if (state1 & STATE_YOU_CAN_PLAY) {
    LOG(INFO) << "Player 1 can play.";
  }

  // TODO(koichi): Add ACK.
}
