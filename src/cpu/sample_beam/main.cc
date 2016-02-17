#include "cpu/sample_beam/beam_search_ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

DECLARE_string(type);

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
  google::InstallFailureSignalHandler();
#endif

  if (FLAGS_type == "2dub") {
    sample::Beam2DubAI().runLoop();
  } else if (FLAGS_type == "full") {
    sample::BeamFullAI().runLoop();
  } else {
    CHECK(false) << "Unknown type: " << FLAGS_type;
  }

  return 0;
}
