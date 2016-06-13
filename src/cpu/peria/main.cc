#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fstream>

#include "cpu/peria/ai.h"
#include "cpu/peria/basic_ai.h"
#include "cpu/peria/pai.h"
#include "cpu/peria/pattern.h"

DECLARE_string(dynamic_pattern);
DEFINE_int32(type, 0, "AI type");

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

  std::unique_ptr<AI> ai;
  switch (FLAGS_type) {
  case 0:
    ai.reset(new peria::Ai(argc, argv));
    break;
  case 1:
    ai.reset(new peria::Pai(argc, argv));
    break;
  case 2:
    ai.reset(new peria::BasicAi(argc, argv));
  }
  CHECK(ai);
  ai->runLoop();

  return 0;
}
