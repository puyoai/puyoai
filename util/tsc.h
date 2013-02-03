#ifndef __TSC_H__
#define __TSC_H__

#include <map>
#include <string>
#include <vector>

// Tsc (Time Stamp Counter) class stacks timestamps in nano seconds.
// Precision of this timer depends on the CPU clock of the machine running on.
class Tsc {
 public:
  Tsc(const std::string& key);
  ~Tsc();
  static void GetStatistics(const std::string& key, double* average, double* rmsd);

 private:
  unsigned long long st_;
  std::string key_;
  static std::map<std::string, std::vector<unsigned long long> > data;
  static void record(const std::string& key, unsigned long long diff);

  // Returns the clock count after the machine boot.
  static unsigned long long rdtsc();
};

#endif  // __TSC_H__
