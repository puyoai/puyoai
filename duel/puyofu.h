#ifndef PUYOFU_H_
#define PUYOFU_H_

#include <memory>
#include <utility>
#include <vector>

class Field;

using namespace std;

class PuyoFu {
 public:
  void setField(int pi, const Field& f, int state, int time);

  void emitFieldTransitionLog(FILE* fp, int pi) const;

 private:
  struct Move {
    int pi;
    auto_ptr<Field> f;
    char next[7];
    int time;
  };

  vector<Move*> moves_;
};

#endif  // PUYOFU_H_
