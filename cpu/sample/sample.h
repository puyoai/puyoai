#include "cpu/cpu_interface.h"

class Decision;

class SampleCpu : public Cpu {
 private:
  virtual void GetDecision(
      const Data& data, Decision* decision, std::string* message);
};
