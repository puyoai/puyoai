#ifndef PUYOFU_H_
#define PUYOFU_H_

#include <stdio.h>

#include <memory>
#include <utility>
#include <vector>

class FieldWithColorSequence;

using namespace std;

class PuyoFu {
 public:
  void setField(int pi, const FieldWithColorSequence& f, int state, int time);

  void emitFieldTransitionLog(FILE* fp, int pi) const;

  bool empty() const { return moves_.empty(); }

 private:
  struct Move {
    int pi;
    auto_ptr<FieldWithColorSequence> f;
    char next[7];
    int time;
  };

  vector<Move*> moves_;
};

#endif  // PUYOFU_H_
