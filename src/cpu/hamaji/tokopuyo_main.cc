#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "solo.h"

DEFINE_int32(seed, -1, "");

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

  if (FLAGS_seed < 0) {
    srand(time(NULL));
    FLAGS_seed = rand();
  }
  LOG(INFO) << "seed=" << FLAGS_seed;

  SoloGame solo(FLAGS_seed, true);
  solo.run();
}
