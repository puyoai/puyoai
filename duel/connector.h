#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include <cstdio>
#include <string>

#include "data.h"

class Connector {
 public:
  Connector();
  Connector(int writer_fd, int reader_fd);

  void Write(const std::string& message);
  bool Read(Data* data);
  int GetReaderFd() const;

 private:
  void Split(char* str, Data* data);
  int writer_fd_;
  int reader_fd_;
  FILE* writer_;
  FILE* reader_;
};

#endif  // __CONNECTOR_H__
