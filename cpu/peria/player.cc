#include "player.h"

#include <algorithm>
#include <cstring>
#include <glog/logging.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "base.h"

void Player::CopyFrom(const Player& player) {
  field_.CopyFrom(player.field());
  state_ = player.state();
  score_ = player.score();
  ojama_ = player.ojama();
  x_ = player.get_x();
  y_ = player.get_y();
  r_ = player.get_r();
}

void Player::SetColorSequence(const string& colors) {
  sequence_.resize(colors.size());
  for (size_t i = 0; i < colors.size(); ++i)
    sequence_[i] = colors[i] - '0';
}

bool operator==(const Player& a, const Player& b) {
  if (a.state() != b.state()) return false;
  if (a.score() != b.score()) return false;
  if (a.ojama() != b.ojama()) return false;
  if (a.get_x() != b.get_x()) return false;
  if (a.get_y() != b.get_y()) return false;
  if (a.get_r() != b.get_r()) return false;

  // Compare visible field
  if (!a.field().EqualTo(b.field(), true /*visible*/))
    return false;

  return true;
}

bool operator!=(const Player& a, const Player& b) {
  return !(a == b);
}

void Player::GetControl(Control* control) {
  vector<Control> controls;
  SearchControls(&controls);

  vector<Player> children;
  double value = 0;
  for (size_t i = 0; i < controls.size(); ++i) {
    Player child(*this);
    child.ApplyControl(controls[i]);
    double val = child.Evaluate();
    if (val > value) {
      value = val;
      *control = controls[i];
    }
  }
}

void Player::ApplyControl(const Control& control) {
  field_.Put(control.first, y_, control.second, sequence_.substr(0, 2));
  int chains = 1, score = 0, frame = 0;
  field_.Simulate(&chains, &score, &frame);
  // Remove controlling puyos.
  sequence_ = sequence_.substr(2);
}

double Player::Evaluate() {
  return 0;
}

namespace {
set<Player::Position>* g_stack = NULL;
set<Player::Position>* g_queue = NULL;

void Insert(int x, int y, int r) {
  Player::Position pos(Player::Control(x, r), y);
  if (g_stack->find(pos) != g_stack->end())
    return;
  g_stack->insert(pos);
  g_queue->insert(pos);
}
}  // namespace

void Player::SearchControls(vector<Control>* controls) {
  set<Position> positions;
  set<Position> queue;
  g_stack = &positions;
  g_queue = &queue;

  queue.insert(Position(Control(x_, r_), y_));
  while (!queue.empty()) {
    set<Position>::iterator itr = queue.begin();
    int x = itr->first.first;
    int r = itr->first.second;
    int y = itr->second;
    queue.erase(itr);

    switch (r) {
    case 0: {
      if (field().IsEmpty(x + 1, y) && field().IsEmpty(x + 1, y + 1))
        Insert(x + 1, y, 0);  // Right
      if (field().IsEmpty(x - 1, y) && field().IsEmpty(x - 1, y + 1))
        Insert(x - 1, y, 0);  // Left
      if (field().IsEmpty(x + 1, y))  // Turn Right
        Insert(x, y, 1);
      else if (field().IsEmpty(x - 1, y))
        Insert(x - 1, y, 1);
      else
        Insert(x, y + 1, 2);
      if (field().IsEmpty(x - 1, y))  // Turn Left
        Insert(x, y, 3);
      else if (field().IsEmpty(x + 1, y))
        Insert(x + 1, y, 3);
    }
    case 1: {
      if (field().IsEmpty(x + 2, y))
        Insert(x + 1, y, 1);  // Right
      if (field().IsEmpty(x - 1, y))
        Insert(x - 1, y, 1);  // Left
      if (field().IsEmpty(x, y - 1))  // Turn Right
        Insert(x, y, 2);
      else if (field().IsEmpty(x, y + 1) && y < Field::kHeight + 2)
        Insert(x - 1, y, 2);
      if (field().IsEmpty(x, y + 1))  // Turn Left
        Insert(x, y, 0);
    }
    case 2: {
      if (field().IsEmpty(x + 1, y) && field().IsEmpty(x + 1, y - 1))
        Insert(x + 1, y, 2);  // Right
      if (field().IsEmpty(x - 1, y) && field().IsEmpty(x - 1, y - 1))
        Insert(x - 1, y, 2);  // Left
      if (field().IsEmpty(x - 1, y))  // Turn Right
        Insert(x, y, 3);
      else if (field().IsEmpty(x + 1, y))
        Insert(x + 1, y, 3);
      else
        Insert(x, y - 1, 0);
      if (field().IsEmpty(x + 1, y))  // Turn Left
        Insert(x, y, 3);
      else if (field().IsEmpty(x - 1, y))
        Insert(x - 1, y, 3);
    }
    case 3: {
      if (field().IsEmpty(x + 1, y))
        Insert(x + 1, y, 3);  // Right
      if (field().IsEmpty(x - 2, y))
        Insert(x - 1, y, 3);  // Left
      if (field().IsEmpty(x, y + 1))  // Turn Right
        Insert(x, y, 0);
      if (field().IsEmpty(x, y - 1))  // Turn Left
        Insert(x, y, 2);
      else if (field().IsEmpty(x + 1, y) && y < Field::kHeight + 2)
        Insert(x, y + 1, 2);
    }
    }
  }

  for (set<Position>::iterator itr = positions.begin();
       itr != positions.end(); ++itr)
    controls->push_back(itr->first);
  sort(controls->begin(), controls->end());
  vector<Control>::iterator itr = unique(controls->begin(), controls->end());
  controls->erase(itr, controls->end());
}
