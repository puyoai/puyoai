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
    kAll = 0x55555555,
  };
  typedef pair<int, int> Control;       // (x, r)
  typedef pair<Control, int> Position;  // ((x, r), y)

  void CopyFrom(const Player& player);
  void SetColorSequence(const string& colors);

  void Search(vector<Player>* children) const;
  void SetOpposite(Player* opposite);

  void SearchControls(int x, int y, int r, vector<Control>* controls) const;

  // Accessors ------------------------------------------------------------
  void set_parent(Player* parent) { parent_ = parent; }
  void set_state(int state) { state_ = state; }
  void set_field(const Field& field) { field_ = field; }
  void set_score(int score) { score_ = score; }
  void set_ojama(int ojama) { ojama_ = ojama; }
  void set_x(int x) { x_ = x; }
  void set_y(int y) { y_ = y; }
  void set_r(int r) { r_ = r; }

  Player* parent() { return parent_; }
  int state() const { return state_; }
  const Field& field() const { return field_; }
  Field* mutable_field() { return &field_; }
  int score() const { return score_; }
  int ojama() const { return ojama_; }
  int get_x() const { return x_; }
  int get_y() const { return y_;}
  int get_r() const { return r_; }

 private:
  Player* parent_;
  int state_;
  Field field_;
  string sequence_;
  int score_;
  int x_;  // x-axis of Jiku puyo
  int y_;  // y-axis of Jiku puyo
  int r_;  // round number of controlling puyo.
  int ojama_;
  Player* opposite_;
};

bool operator==(const Player& a, const Player& b);
bool operator!=(const Player& a, const Player& b);

#endif  // CPU_PERIA_PLAYER_H_
