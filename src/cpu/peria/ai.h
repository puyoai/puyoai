#ifndef PERIA_AI_H_
#define PERIA_AI_H_

#include <string>

#include "cpu/peria/connector.h"

namespace peria {

class Ai {
 public:
  Ai();
  ~Ai();

  void RunLoop();

 private:
  std::string name_;
  Connector connector_;
};

}  // namespace peria

#endif
