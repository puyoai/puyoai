#ifndef DUEL_CONNECTOR_H_
#define DUEL_CONNECTOR_H_

#include <cstdio>
#include <string>

#include "core/data.h"

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

#endif  // DUEL_CONNECTOR_H_
