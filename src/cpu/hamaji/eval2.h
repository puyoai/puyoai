#ifndef HAMAJI_EVAL2_H_
#define HAMAJI_EVAL2_H_

#include <string>
#include <vector>

#include "../../core/decision.h"

#include "base.h"

class LF;
class LP;

class Eval2 {
public:
  Eval2();
  ~Eval2();

  double eval(LP* plan);

private:
};

#endif  // HAMAJI_EVAL2_H_
