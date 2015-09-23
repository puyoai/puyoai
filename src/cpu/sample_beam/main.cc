#include "cpu/sample_beam/beam_search_ai.h"

// #include <array>
// #include <limits>
// #include <sstream>
// #include <vector>

#include <gflags/gflags.h>
// #include <glog/logging.h>

// #include "base/base.h"
// #include "base/time.h"
// #include "core/plan/plan.h"
// #include "core/rensa/rensa_detector.h"
// #include "core/core_field.h"
// #include "core/kumipuyo_seq_generator.h"

DECLARE_string(type);

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  if (FLAGS_type == "2dub") {
    sample::Beam2DubAI().runLoop();
  } else if (FLAGS_type == "full") {
    sample::BeamFullAI().runLoop();
  } else {
    CHECK(false) << "Unknown type: " << FLAGS_type;
  }

  return 0;
}
