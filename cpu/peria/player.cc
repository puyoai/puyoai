#include "player.h"

#include <cstring>
#include <glog/logging.h>
#include <sstream>
#include <string>

#include "base.h"

void Player::CopyFrom(const Player& player) {
  for (int i = 1; i <= Field::kWidth; ++i) {
    for (int j = 1; j <= Field::kHeight; ++j) {
      this->field.Set(i, j, field.Get(i, j));
    }
  }
  state = player.state;
  score = player.score;
  ojama = player.ojama;
  x = player.x;
  y = player.y;
  r = player.r;
}

bool operator==(const Player& a, const Player& b) {
  if (a.state != b.state) return false;
  if (a.score != b.score) return false;
  if (a.ojama != b.ojama) return false;
  if (a.x != b.x) return false;
  if (a.y != b.y) return false;
  if (a.r != b.r) return false;

  // Compare visible field
  if (!a.field.EqualTo(b.field, true))
    return false;

  return true;
}

bool operator!=(const Player& a, const Player& b) {
  return !(a == b);
}
