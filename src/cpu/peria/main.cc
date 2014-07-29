#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fstream>

#include "cpu/peria/ai.h"
#include "cpu/peria/pattern.h"

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  //std::ifstream pattern_file("cpu/peria/book.txt");
  //if (pattern_file.is_open())
  //  peria::Pattern::ReadBook(pattern_file);

  peria::Ai().runLoop();

  return 0;
}
