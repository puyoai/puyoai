#ifndef CORE_DECISION_H_
#define CORE_DECISION_H_

class Decision {
 public:
  static const Decision NO_INPUT;
  static const Decision USE_LAST_INPUT;

 public:
  // X of the JIKU-PUYO. (1<=x<=6)
  int x;

  // JIKU-PUYO=X KO-PUYO=Y
  // 0:  1:    2:  3:
  //  Y   X Y   X   Y X
  //  X         Y
  int r;

  Decision() : x(0), r(0) {}
  Decision(int x0, int r0) : x(x0), r(r0) {}

  bool IsValid() const {
    if (x == 1 && r == 3) {
      return false;
    }
    if (x == 6 && r == 1) {
      return false;
    }
    return (1 <= x && x <= 6 && 0 <= r && r <= 3);
  }

  bool operator==(Decision d) const {
    return x == d.x && r == d.r;
  }
  bool operator!=(Decision d) const {
    return x != d.x || r != d.r;
  }
};

#endif  // CORE_DECISION_H_
