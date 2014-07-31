#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fstream>

#include "cpu/peria/ai.h"
#include "cpu/peria/pattern.h"

DEFINE_string(pattern, "", "Figures a template file name.");

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::ifstream pattern_file(FLAGS_pattern);
  if (pattern_file.is_open()) {
    LOG(INFO) << "Leading " << FLAGS_pattern << " for pattern mathcing";
    peria::Pattern::ReadBook(pattern_file);
  } else {
    LOG(INFO) << "Failed in loading " << FLAGS_pattern;
  }

  peria::Ai().runLoop();

  return 0;
}
