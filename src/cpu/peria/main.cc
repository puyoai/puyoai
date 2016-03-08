#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fstream>

#include "cpu/peria/ai.h"
#include "cpu/peria/pai.h"
#include "cpu/peria/pattern.h"

DECLARE_string(pattern);
DEFINE_int32(type, 0, "AI type");

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
  google::InstallFailureSignalHandler();
#endif

  std::ifstream pattern_file(FLAGS_pattern);
  if (pattern_file.is_open()) {
    LOG(INFO) << "Leading " << FLAGS_pattern << " for pattern mathcing";
    peria::Pattern::ReadBook(pattern_file);
    peria::Pattern::BuildCombination();
  } else {
    LOG(INFO) << "Failed in loading " << FLAGS_pattern;
  }

  std::unique_ptr<AI> ai;
  switch (FLAGS_type) {
  case 0:
    ai.reset(new peria::Ai(argc, argv));
    break;
  case 1:
    ai.reset(new peria::Pai(argc, argv));
    break;
  }
  CHECK(ai);
  ai->runLoop();

  return 0;
}
