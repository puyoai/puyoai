#ifndef PERIA_AI_H_
#define PERIA_AI_H_

#include <string>

#include "cpu/peria/player.h"

namespace peria {

class Ai {
 public:
  Ai();
  ~Ai();

  void RunLoop();

 private:
  std::string name_;
};

}  // namespace peria

#endif
