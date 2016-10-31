#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fstream>

#include "cpu/peria/ai.h"

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
  google::InstallFailureSignalHandler();
#endif

  std::unique_ptr<AI> ai(new peria::Ai(argc, argv));
  ai->runLoop();

  return 0;
}
