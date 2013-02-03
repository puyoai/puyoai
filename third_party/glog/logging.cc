// This file is a workaround for compiling core/* without installing glog.
// The content of this file should be replaced with the real glog.

#include "logging.h"
#include <fstream>

namespace google {
  void InitGoogleLogging(const char* arg0) {}
  void InstallFailureSignalHandler() {}
  std::ofstream null_stream("/dev/null");
  void FlushLogFiles(int type) {}
}  // namespace google
