#include <gflags/gflags.h>
#include <glog/logging.h>

#include "cpu/peria/ai.h"

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  peria::Ai().runLoop();

  return 0;
}
