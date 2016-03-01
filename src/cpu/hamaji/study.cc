#include "build/build_config.h"

#include "eval.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

#ifdef OS_LINUX
  FLAGS_logtostderr = true;
#endif

  Eval eval;
  eval.study();
}
