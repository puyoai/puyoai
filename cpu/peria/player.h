#ifndef CPU_PERIA_PLAYER_H_
#define CPU_PERIA_PLAYER_H_

#include <vector>

#include "base.h"
#include "field.h"

class Player {
 public:
  enum State {
    kNone = 0,
    kPlay = 1 << 0,
    kNext2 = 1 << 2,
    kSet = 1 << 4,
    kWin = 1 << 6,
    kChainEnd = 1 << 8,
    kAll = 0x55555555u,
  };
  typedef pair<int, int> Control;
  typedef pair<Control, int> Position;

  void CopyFrom(const Player& player);
  void Search(vector<Player>* children) const;

  void SearchControls(int x, int y, int r, vector<Control>* controls) const;

  void set_parent(Player* parent) { parent_ = parent; }
  Player* parent() { return parent_; }
  void set_state(int state) { state_ = state; }
  int state() const { return state_; }
  void set_field(const Field& field) { field_ = field; }
  const Field& field() const { return field_; }
  Field* mutable_field() { return &field_; }
  void set_score(int score) { score_ = score; }
  int score() const { return score_; }
  void set_ojama(int ojama) { ojama_ = ojama; }
  int ojama() const { return ojama_; }
  void set_x(int x) { x_ = x; }
  int get_x() const { return x_; }
  void set_y(int y) { y_ = y; }
  int get_y() const { return y_;}
  void set_r(int r) { r_ = r; }
  int get_r() const { return r_; }

 private:
  Player* parent_;
  int state_;
  Field field_;
  int score_;
  int x_;  // x-axis of Jiku puyo
  int y_;  // y-axis of Jiku puyo
  int r_;  // round number of controlling puyo.
  int ojama_;
};

bool operator==(const Player& a, const Player& b);
bool operator!=(const Player& a, const Player& b);

#endif  // CPU_PERIA_PLAYER_H_
