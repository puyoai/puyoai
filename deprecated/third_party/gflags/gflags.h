#ifndef THIRD_PARTY_GFLAGS_GFLAGS_H_
#define THIRD_PARTY_GFLAGS_GFLAGS_H_

#define DEFINE_bool(name, val, txt) bool FLAGS_##name = val
#define DEFINE_int32(name, val, txt) int FLAGS_##name = val
#define DEFINE_string(name, val, txt) string FLAGS_##name = val
#define DEFINE_double(name, val, txt) double FLAGS_##name = val

namespace google {
  void ParseCommandLineFlags(int* argc, char** argv[], bool b);
}  // namespace google

extern bool FLAGS_logtostderr;

#endif  // THIRD_PARTY_GFLAGS_GFLAGS_H_
