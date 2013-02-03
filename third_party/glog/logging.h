// This file is a workaround for compiling core/* without installing glog.
// The content of this file should be replaced with the real glog.

#ifndef __GLOG_LOGGING_FAKE_H__
#define __GLOG_LOGGING_FAKE_H__

#include <iostream>
#include <fstream>

#define LOG(type) if(0) google::null_stream << ""
#define VLOG(x) if(x) google::null_stream << ""
#define CHECK(x) if (!(x)) google::null_stream << ""
#define DCHECK(x) if ((x)) google::null_stream << ""
#define CHECK_EQ(x, y) if((x) == (y)) google::null_stream << ""
#define CHECK_NE(x, y) if((x) != (y)) google::null_stream << ""
#define CHECK_GE(x, y) if((x) > (y)) google::null_stream << ""
#define CHECK_LE(x, y) if((x) < (y)) google::null_stream << ""

namespace google {
  void InitGoogleLogging(const char* arg0);
  void InstallFailureSignalHandler();
  extern std::ofstream null_stream;
  void FlushLogFiles(int type);

  static const int INFO = 1;
}  // namespace google

#endif  // __GLOG_LOGGING_FAKE_H__
