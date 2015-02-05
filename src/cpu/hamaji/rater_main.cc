#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "rater.h"
#include "ratingstats.h"

DEFINE_int32(eval_cnt, 100,
             "Run the game this times and show some stats");
DEFINE_int32(eval_threads, 1, "");
DEFINE_int32(seed, -1, "");

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

  if (FLAGS_seed < 0) {
    srand(time(NULL));
    FLAGS_seed = rand();
  }
  LOG(INFO) << "seed=" << FLAGS_seed;

  Rater rater(FLAGS_eval_threads, FLAGS_eval_cnt, FLAGS_seed);
  RatingStats all_stats;
  rater.eval(&all_stats);
  all_stats.Print();
}
