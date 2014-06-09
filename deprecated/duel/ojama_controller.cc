#include "ojama_controller.h"

#include <cstdlib>
#include <vector>

using namespace std;

OjamaController OjamaController::DUMMY;

OjamaController::OjamaController() {
  opponent_ = &DUMMY;
  fixed_ = 0;
  pending_ = 0;
}
void OjamaController::SetOpponent(OjamaController* opponent) {
  opponent_ = opponent;
}
int OjamaController::GetFixedOjama() const {
  return fixed_;
}
int OjamaController::GetPendingOjama() const {
  return pending_;
}
void OjamaController::Send(int num) {
  if (fixed_ >= num) {
    fixed_ -= num;
  } else if (fixed_ + pending_ > num) {
    pending_ -= (num - fixed_);
    fixed_ = 0;
  } else {
    num -= fixed_ + pending_;
    fixed_ = 0;
    pending_ = 0;
    if (opponent_) {
      opponent_->pending_ += num;
    }
  }
}
void OjamaController::Fix() {
  if (opponent_) {
    opponent_->fixed_ += opponent_->pending_;
    opponent_->pending_ = 0;
  }
}
// Returns what column should drop how many Ojama puyos. The returned vector
// has 6 elements.
// If there are more than 30 Ojama puyos to drop, all column will have 5.
vector<int> OjamaController::Drop() {
  int drop_ojama = (fixed_ >= 30) ? 30 : fixed_;
  fixed_ -= drop_ojama;

  // Decide which column to drop.
  int positions[6] = {0};
  for (int i = 0; i < 6; i++) {
    positions[i] = i;
  }
  for (int i = 1; i < 6; i++) {
    swap(positions[i], positions[rand() % (i+1)]);
  }

  vector<int> ret(6, 0);
  int lines = drop_ojama / 6;
  drop_ojama %= 6;
  for (int i = 0; i < drop_ojama; i++) {
    ret[positions[i]] = lines + 1;
  }
  for (int i = drop_ojama; i < 6; i++) {
    ret[positions[i]] = lines;
  }

  return ret;
}
