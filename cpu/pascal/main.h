#include "cpu_interface.h"

class Decision;

class PascalCpu : public Cpu {
 private:
  virtual void GetDecision(
      const Data& data, Decision* decision, std::string* message);
};
