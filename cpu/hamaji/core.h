#ifndef CORE_H_
#define CORE_H_

#include <string>

#include "../../core/decision.h"

#include "base.h"

class Eval;
class Game;

class Core {
public:
  explicit Core(bool is_solo);
  ~Core();

  Decision decide(Game* game);

  void resetBest() { best_chain_ = best_score_ = 0; }
  int best_chain() const { return best_chain_; }
  int best_score() const { return best_score_; }

  const string& msg() const { return msg_; }

private:
  Eval* eval_;
  bool is_solo_;

  int best_chain_;
  int best_score_;

  string msg_;
};

#endif  // CORE_H_
