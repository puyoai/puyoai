#include "field_realtime.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <glog/logging.h>
#include <sstream>
#include <string>

#include "core/ctrl.h"
#include "core/state.h"
#include "duel/ojama_controller.h"

using namespace std;

FieldRealtime::FieldRealtime(int player_id, const string& color_sequence,
                             OjamaController* ojama_ctrl)
    : Field() {
  Init();
  player_id_ = player_id;
  SetColorSequence(color_sequence);
  ojama_ctrl_ = ojama_ctrl;
}

void FieldRealtime::Init() {
  delay_double_next_ = getenv("PUYO_DELAY_DOUBLE_NEXT") != NULL;

  PrepareNextPuyo();

  next_puyo_ = 0;
  simulate_real_state_ = STATE_USER;
  sleep_for_ = 0;
  is_dead_ = false;
  field_state_ = 1;
  frames_for_free_fall_ = 0;
  score_ = 0;
  consumed_score_ = 0;
  ojama_position_ = vector<int>(6, 0);
  ojama_dropping_ = false;
  current_chains_ = 1;
  quickturn_ = 0;
  is_zenkesi_ = false;
  dropped_rows_ = 0;
  yokoku_delay_ = 0;
  sent_wnext_appeared_ = false;
  drop_animation_ = false;
}

bool FieldRealtime::TryChigiri() {
  if (Chigiri()) {
    dropped_rows_++;
    sleep_for_ =
        (dropped_rows_ == 1) ? FRAMES_CHIGIRI_1_LINE_1 :
        (dropped_rows_ == 2) ? FRAMES_CHIGIRI_1_LINE_2 :
        FRAMES_CHIGIRI_1_LINE_3;

    drop_animation_ = true;
    return true;
  } else {
    dropped_rows_ = 0;
    simulate_real_state_ = STATE_VANISH;
    if (drop_animation_) {
      sleep_for_ = FRAMES_AFTER_CHIGIRI;
      drop_animation_ = false;
    } else {
      sleep_for_ = FRAMES_AFTER_NO_CHIGIRI;
    }
    return false;
  }
}

bool FieldRealtime::TryVanish() {
  int score = 0;
  if (Vanish(current_chains_, &score)) {
    current_chains_++;
    score_ += score;
    if (is_zenkesi_) {
      score_ += ZENKESI_BONUS;
      is_zenkesi_ = false;
    }
    simulate_real_state_ = STATE_DROP;
    sleep_for_ = FRAMES_VANISH_ANIMATION;

    // Set Yokoku Ojama.
    if ((score_ - consumed_score_ >= SCORE_FOR_OJAMA) && (current_chains_ > 1)) {
      int attack_ojama = (score_ - consumed_score_) / SCORE_FOR_OJAMA;
      ojama_ctrl_->Send(attack_ojama);
      consumed_score_ = score_ / SCORE_FOR_OJAMA * SCORE_FOR_OJAMA;
    }
    return true;
  } else {
    sleep_for_ = FRAMES_AFTER_VANISH;
    FinishChain();
    return false;
  }
}

void FieldRealtime::FinishChain() {
  is_dead_ = (Get(3, 12) != EMPTY);
  if (!is_zenkesi_) {
    is_zenkesi_ = true;
    for (int i = 1; i <= 6; i++) {
      if (Get(i, 1) != EMPTY) {
        is_zenkesi_ = false;
      }
    }
  }

  ojama_ctrl_->Fix();
  if (ojama_ctrl_->GetFixedOjama() > 0) {
    simulate_real_state_ = STATE_OJAMA;
  } else {
    PrepareNextPuyo();
  }
  current_chains_ = 1;
  Clean();
  field_state_ |= STATE_CHAIN_DONE;
}

bool FieldRealtime::TryDrop() {
  if (Drop1line()) {
    sleep_for_ = FRAMES_DROP_1_LINE;
    drop_animation_ = true;
    return true;
  } else {
    Clean();
    if (drop_animation_) {
      sleep_for_ = FRAMES_AFTER_DROP;
      drop_animation_ = false;
      simulate_real_state_ = STATE_VANISH;
    } else {
      FinishChain();
      sleep_for_ = FRAMES_AFTER_NO_DROP;
    }
    return false;
  }
}

bool FieldRealtime::TryOjama(PlayerLog* player_log) {
  if (!ojama_dropping_) {
    ojama_position_ = ojama_ctrl_->Drop();
    for (int i = 0; i < 6; i++) {
      if (ojama_position_[i]) {
        ojama_dropping_ = true;
      }
    }
    // When Ojama puyo starts to fall.
    if (ojama_dropping_) {
      player_log->execution_data.ojama = ojama_position_;
    }
  }

  if (ojama_dropping_) {
    for (int i = 0; i < 6; i++) {
      if (ojama_position_[i] > 0) {
        if (Get(i + 1, 13) == EMPTY) {
          Set(i + 1, 13, OJAMA);
        }
        ojama_position_[i]--;
      }
    }
  }
  if (Drop1line()) {
    dropped_rows_++;
    sleep_for_ =
        (dropped_rows_ == 1) ? FRAMES_CHIGIRI_1_LINE_1 :
        (dropped_rows_ == 2) ? FRAMES_CHIGIRI_1_LINE_2 :
        FRAMES_CHIGIRI_1_LINE_3;

    return true;
  } else {
    dropped_rows_ = 0;
    ojama_dropping_ = false;
    is_dead_ = (Get(3, 12) != EMPTY);
    sleep_for_ = FRAMES_AFTER_DROP;
    field_state_ |= STATE_OJAMA_DROPPED;
    PrepareNextPuyo();
    return false;
  }
}

// Returns true if a key input is accepted.
bool FieldRealtime::Play(Key key, PlayerLog* player_log) {
  field_state_ = STATE_NONE;
  if (quickturn_ > 0) {
    quickturn_--;
  }

  if (simulate_real_state_ == STATE_USER) {
    frames_for_free_fall_++;
  }

  if (yokoku_delay_ > 0) {
    yokoku_delay_--;
  }

  if (yokoku_delay_ == 0 && !sent_wnext_appeared_) {
    field_state_ |= STATE_WNEXT_APPEARED;
    sent_wnext_appeared_ = true;
  }

  // Loop until some functionality consumes this frame.
  while (true) {
    if (sleep_for_ > 0) {
      sleep_for_--;
      // Player can send a command in the next frame.
      if (simulate_real_state_ == STATE_USER && sleep_for_ == 0) {
        field_state_ |= STATE_YOU_CAN_PLAY;
      }
      return false;
    }

    if (simulate_real_state_ == STATE_CHIGIRI) {
      if (TryChigiri()) {
        return false;
      } else {
        continue;
      }
    }
    if (simulate_real_state_ == STATE_VANISH) {
      if (TryVanish()) {
        return false;
      } else {
        continue;
      }
    }
    if (simulate_real_state_ == STATE_DROP) {
      if (TryDrop()) {
        return false;
      } else {
        continue;
      }
    }
    if (simulate_real_state_ == STATE_OJAMA) {
      if (TryOjama(player_log)) {
        return false;
      } else {
        continue;
      }
    }

    if (simulate_real_state_ == STATE_USER) {
      bool accepted = true;
      bool grounded = PlayInternal(key, &accepted);
      if (!grounded) {
        if (accepted) {
          sleep_for_ = FRAMES_AFTER_USER_INTERACTION;
        }
        if (frames_for_free_fall_ >= FRAMES_FREE_FALL) {
          bool dummy;
          grounded = PlayInternal(KEY_DOWN, &dummy);
        }
      }

      if (grounded) {
        chigiri_x_ = -1;
        chigiri_y_ = -1;
        simulate_real_state_ = STATE_CHIGIRI;
        field_state_ |= STATE_YOU_GROUNDED;
      }

      if (key == KEY_DOWN && accepted) {
        score_++;
      }
      return accepted;
    }
  }  // end while
}

// returns true if the puyo grounded.
bool FieldRealtime::PlayInternal(Key key, bool* accepted) {
  bool ground = false;

  int x1, x2, y1, y2;
  char c1, c2;
  int r;
  GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);

  switch (key) {
    case KEY_RIGHT_TURN:
      switch (r_) {
        case 0:
          if (Get(x_ + 1, y_) == EMPTY) {
            r_ = (r_ + 1) % 4;
            *accepted = true;
          } else if (Get(x_ - 1, y_) == EMPTY) {
            r_ = (r_ + 1) % 4;
            x_--;
            *accepted = true;
          } else {
            if (quickturn_ > 0) {
              r_ = 2;
              y_++;
              *accepted = true;
              quickturn_ = 0;
            } else {
              quickturn_ = FRAMES_QUICKTURN;
              *accepted = true;
            }
          }
          break;
        case 1:
          if (Get(x_, y_ - 1) == EMPTY) {
            r_ = (r_ + 1) % 4;
            *accepted = true;
          } else {
            r_ = (r_ + 1) % 4;
            y_++;
            *accepted = true;
          }
          break;
        case 2:
          if (Get(x_ - 1, y_) == EMPTY) {
            r_ = (r_ + 1) % 4;
            *accepted = true;
          } else if (Get(x_ + 1, y_) == EMPTY) {
            r_ = (r_ + 1) % 4;
            x_++;
            *accepted = true;
          } else {
            if (quickturn_ > 0) {
              r_ = 0;
              y_--;
              *accepted = true;
              quickturn_ = 0;
              *accepted = true;
            } else {
              quickturn_ = FRAMES_QUICKTURN;
              *accepted = true;
            }
          }
          break;
        case 3:
          r_ = (r_ + 1) % 4;
          *accepted = true;
          break;
      }
      return false;
    case KEY_LEFT_TURN:
      switch (r_) {
        case 0:
          if (Get(x_ - 1, y_) == EMPTY) {
            r_ = (r_ + 3) % 4;
            *accepted = true;
          } else if (Get(x_ + 1, y_) == EMPTY) {
            r_ = (r_ + 3) % 4;
            x_++;
            *accepted = true;
          } else {
            if (quickturn_ > 0) {
              r_ = 2;
              y_++;
              *accepted = true;
              quickturn_ = 0;
            } else {
              quickturn_ = FRAMES_QUICKTURN;
              *accepted = true;
            }
          }
          break;
        case 1:
          r_ = (r_ + 3) % 4;
          *accepted = true;
          break;
        case 2:
          if (Get(x_ + 1, y_) == EMPTY) {
            r_ = (r_ + 3) % 4;
            *accepted = true;
          } else if (Get(x_ - 1, y_) == EMPTY) {
            r_ = (r_ + 3) % 4;
            x_--;
            *accepted = true;
          } else {
            if (quickturn_ > 0) {
              r_ = 0;
              y_--;
              *accepted = true;
              quickturn_ = 0;
            } else {
              quickturn_ = FRAMES_QUICKTURN;
              *accepted = true;
            }
          }
          break;
        case 3:
          if (Get(x_, y_ - 1) == EMPTY) {
            r_ = (r_ + 3) % 4;
            *accepted = true;
          } else {
            r_ = (r_ + 3) % 4;
            y_++;
            *accepted = true;
          }
          break;
      }
      return false;
    case KEY_RIGHT:
      if (Get(x1 + 1, y1) == EMPTY &&
          Get(x2 + 1, y2) == EMPTY) {
        x_++;
        *accepted = true;
      } else {
        *accepted = false;
      }
      break;
    case KEY_LEFT:
      if (Get(x1 - 1, y1) == EMPTY &&
          Get(x2 - 1, y2) == EMPTY) {
        x_--;
        *accepted = true;
      } else {
        *accepted = false;
      }
      break;
    case KEY_DOWN:
      frames_for_free_fall_ = 0;
      if (Get(x1, y1 - 1) == EMPTY &&
          Get(x2, y2 - 1) == EMPTY) {
        y_--;
        *accepted = true;
      } else {
        // Ground.
        Set(x1, y1, GetNextPuyo(0));
        Set(x2, y2, GetNextPuyo(1));
        *accepted = false;
        ground = true;
      }
      break;
    case KEY_NONE:
      *accepted = false;
      break;
  }

  return ground;
}

bool FieldRealtime::Chigiri() {
  if (chigiri_x_ < 0) {
    int x1, x2, y1, y2;
    char c1, c2;
    int r;
    GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);
    if (Get(x1, y1 - 1) == EMPTY) {
      chigiri_x_ = x1;
      chigiri_y_ = y1;
    }
    if (Get(x2, y2 - 1) == EMPTY) {
      chigiri_x_ = x2;
      chigiri_y_ = y2;
    }
  }
  if (chigiri_x_ < 0) {
    return false;
  } else {
    int x = chigiri_x_;
    int y = chigiri_y_;
    if (Get(x, y - 1) == EMPTY) {
      Set(x, y - 1, Get(x, y));
      Set(x, y, EMPTY);
      chigiri_y_--;
      return true;
    } else {
      return false;
    }
  }
}

bool FieldRealtime::Drop1line() {
  bool ret = false;
  for (int x = 1; x <= WIDTH; x++) {
    // Puyo in 14th row will not drop to 13th row. If there is a puyo on
    // 14th row, it'll stay there forever. This behavior is a famous bug in
    // Puyo2.
    for (int y = 1; y < MAP_HEIGHT - 3; y++) {
      if (Get(x, y) == EMPTY && Get(x, y + 1) != EMPTY) {
        Set(x, y, Get(x, y + 1));
        Set(x, y + 1, EMPTY);

        ret = true;
      }
    }
  }
  return ret;
}

void FieldRealtime::PrepareNextPuyo() {
  x_ = 3;
  y_ = 12;
  r_ = 0;

  next_puyo_ += 2;
  if (delay_double_next_) {
    yokoku_delay_ = FRAMES_YOKOKU_DELAY;
  }
  sent_wnext_appeared_ = false;
  simulate_real_state_ = STATE_USER;
}

void FieldRealtime::GetCurrentPuyo(int* x1, int* y1, char* c1,
                                   int* x2, int* y2, char* c2, int* r) const {
  *x1 = x_;
  *y1 = y_;
  *c1 = GetNextPuyo(0);
  *x2 = x_ + (r_ == 1) - (r_ == 3);
  *y2 = y_ + (r_ == 0) - (r_ == 2);
  *c2 = GetNextPuyo(1);
  *r = r_;
}

string Locate(int x, int y) {
  stringstream ss;
  ss << "\x1b[" << y << ";" << x << "H";
  return ss.str();
}

void FieldRealtime::Print() const {
  Print("");
}

string GetPuyoText(char color, int y = 0) {
  const string C_RED = "\x1b[41m";
  const string C_BLUE = "\x1b[44m";
  const string C_GREEN = "\x1b[42m";
  const string C_YELLOW = "\x1b[43m";
  const string C_BLACK = "\x1b[49m";

  string text;
  if (color == OJAMA) {
    text = "@@";
  } else if (color == WALL) {
    text = "##";
  } else {
    if (y == 13)
      text = "__";
    else
      text = "  ";
  }

  string color_code;
  switch (color) {
    case RED: color_code = C_RED; break;
    case BLUE: color_code = C_BLUE; break;
    case GREEN: color_code = C_GREEN; break;
    case YELLOW: color_code = C_YELLOW; break;
    default: color_code = C_BLACK; break;
  }

  return color_code + text + C_BLACK;
}

void FieldRealtime::Print(const string& debug_message) const {
  int x1, y1, x2, y2, r;
  char c1, c2;
  GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);

  int pos_x = 1 + 30 * player_id_;
  int pos_y = 1;
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      cout << Locate(pos_x + x * 2, pos_y + MAP_HEIGHT - y);

      char color = Get(x, y);
      if (simulate_real_state_ == STATE_USER) {
        if (x == x1 && y == y1) {
          color = c1;
        }
        if (x == x2 && y == y2) {
          color = c2;
        }
      }

      cout << GetPuyoText(color, y);
    }
  }

  // Ojama puyo info
  cout << Locate(pos_x, pos_y) << ojama_ctrl_->GetFixedOjama()
       << "(" << ojama_ctrl_->GetPendingOjama() << ")          ";
  // Next puyo info
  for (int i = 2; i < 6; i++) {
    const string location =
        Locate(pos_x + 9 * 2, pos_y + 3 + (i - 2) + ((i - 2) / 2));
    if (yokoku_delay_ > 0 && i >= 4) {
      cout << location << "  ";
    } else {
      cout << location << GetPuyoText(GetNextPuyo(i));
    }
  }

  // Score
  cout << Locate(pos_x, pos_y + MAP_HEIGHT + 1) << setw(10) << score_;
  // Debug message ("\x1B[0K" clears the line)
  if (!debug_message.empty()) {
    cout << Locate(1, pos_y + MAP_HEIGHT + 3 + player_id_)
	 << "\x1B[0K" << debug_message << flush;
  }
  cout << Locate(1, pos_y + MAP_HEIGHT + 5) << flush;
}

bool FieldRealtime::IsDead() const {
  return is_dead_;
}

string FieldRealtime::GetFieldInfo() const {
  stringstream ss;
  for (int y = 12; y >= 1; y--) {
    for (int x = 1; x <= 6; x++) {
      switch (Get(x, y)) {
        case EMPTY: ss << '0'; break;
        case OJAMA: ss << '1'; break;
        case WALL: ss << '2'; break;
        case RED: ss << '4'; break;
        case BLUE: ss << '5'; break;
        case YELLOW: ss << '6'; break;
        case GREEN: ss << '7'; break;
        default: ss << '?'; break;
      }
    }
  }
  return ss.str();
}

string FieldRealtime::GetYokokuInfo() const {
  stringstream ss;
  for (int i = 0; i < 6; i++) {
    if (yokoku_delay_ > 0 && i >= 4) {
      ss << '0';
    } else {
      switch (GetNextPuyo(i)) {
        case EMPTY: ss << '0'; break;
        case OJAMA: ss << '1'; break;
        case WALL: ss << '2'; break;
        case RED: ss << '4'; break;
        case BLUE: ss << '5'; break;
        case YELLOW: ss << '6'; break;
        case GREEN: ss << '7'; break;
        default: ss << '?'; break;
      }
    }
  }
  return ss.str();
}

int FieldRealtime::GetStateInfo() const {
  return field_state_;
}

int FieldRealtime::GetScore() const {
  return score_;
}

Key FieldRealtime::GetKey(const Decision& decision) {
  if (!decision.IsValid()) {
    return KEY_NONE;
  }

  int x1, y1, x2, y2, r;
  char c1, c2;
  GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);

  LOG(INFO) << "[" << x1 << ", " << y1 << "(" << r << ")] -> ["
            << decision.x << "(" << decision.r << ")]";

  vector<KeyTuple> keys;
  KeyTuple next_key;
  if (Ctrl::getControlOnline(
          *this, KumipuyoPos(decision.x, 1, decision.r),
          KumipuyoPos(x1, y1, r), &keys)) {
    LOG(INFO) << Ctrl::buttonsDebugString(keys);

    // Remove redundant key stroke.
    if (r == 3 && keys[0].b1 == KEY_RIGHT_TURN && keys[1].b1 == KEY_LEFT_TURN) {
      next_key = keys[2];
    } else if (r == 1 && keys[0].b1 == KEY_LEFT_TURN &&
               keys[1].b1 == KEY_RIGHT_TURN) {
      next_key = keys[2];
    } else {
      next_key = keys[0];
    }
  } else {
    LOG(INFO) << "No way...";
    return KEY_NONE;
  }

  if (next_key.b1 != KEY_NONE) {
    return next_key.b1;
  } else if (next_key.b2 != KEY_NONE) {
    return next_key.b2;
  } else {
    return KEY_NONE;
  }
}

int FieldRealtime::GetFixedOjama() const {
  return ojama_ctrl_->GetFixedOjama();
}

int FieldRealtime::GetPendingOjama() const {
  return ojama_ctrl_->GetPendingOjama();
}

// Testing only.
FieldRealtime::SimulationState FieldRealtime::GetSimulationState() const {
  if (sleep_for_) {
    return STATE_SLEEP;
  } else {
    return simulate_real_state_;
  }
}
