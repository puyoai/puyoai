#include <gflags/gflags.h>
#include <glog/logging.h>

#include "ai.h"

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  munetoshi::AI(argc, argv).runLoop();

  return 0;
}
