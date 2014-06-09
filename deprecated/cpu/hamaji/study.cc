#include "eval.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

int main(int argc, char* argv[]) {
#ifdef OS_LINUX
  InitGoogle(argv[0], &argc, &argv, true);
#else
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);
#endif

#ifdef __linux__
  FLAGS_logtostderr = true;
#endif

  Eval eval;
  eval.study();
}
