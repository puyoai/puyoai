#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fstream>

#include "cpu/peria/ai.h"
#include "cpu/peria/pattern.h"

DECLARE_string(dynamic_pattern);

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
  google::InstallFailureSignalHandler();
#endif

  std::ifstream dynamic_pattern_file(FLAGS_dynamic_pattern);
  if (dynamic_pattern_file.is_open()) {
    LOG(INFO) << "Leading " << FLAGS_dynamic_pattern << " for pattern mathcing";
    peria::DynamicPatternBook::readBook(dynamic_pattern_file);
  } else {
    LOG(INFO) << "Failed in loading " << FLAGS_dynamic_pattern;
  }

  std::unique_ptr<AI> ai(new peria::Ai(argc, argv));
  ai->runLoop();

  return 0;
}
