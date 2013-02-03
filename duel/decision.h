#ifndef __DECISION_H__
#define __DECISION_H__

class Decision {
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

  static const Decision NO_INPUT;
  static const Decision USE_LAST_INPUT;

  bool IsValid() const;
  bool operator == (const Decision& d) const;
};

#endif  // __DECISION_H__
