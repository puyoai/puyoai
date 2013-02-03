#include "connector.h"

#include <string.h>

#include <cstdio>
#include <sstream>
#include <string>

Connector::Connector()
  : writer_fd_(0),
    reader_fd_(0),
    writer_(NULL),
    reader_(NULL) {
}

Connector::Connector(int writer_fd, int reader_fd) {
  writer_fd_ = writer_fd;
  reader_fd_ = reader_fd;
  writer_ = fdopen(writer_fd, "w");
  reader_ = fdopen(reader_fd, "r");
}

void Connector::Write(const std::string& message) {
  if (!writer_)
    return;
  std::fprintf(writer_, "%s\n", message.c_str());
  std::fflush(writer_);
}

bool Connector::Read(Data* data) {
  if (!reader_)
    return false;

  char buf[1000];
  char* ptr = fgets(buf, 999, reader_);
  if (ptr) {
    size_t len = strlen(ptr);
    if (ptr[len-1] == '\n') {
      ptr[--len] = '\0';
    }
    if (ptr[len-1] == '\r') {
      ptr[--len] = '\0';
    }
    Split(buf, data);
    return true;
  } else {
    return false;
  }
}

void Connector::Split(char* str, Data* data) {
  std::istringstream iss(str);
  std::string tmp;

  data->original = std::string(str);
  data->original = data->original.substr(0, data->original.size() - 1);
  data->x = 0;
  data->r = 0;
  while(getline(iss, tmp, ' ')) {
    if (tmp.substr(0, 3) == "ID=") {
      std::istringstream istr(tmp.c_str() + 3);
      istr >> data->id;
    } else if (tmp.substr(0, 2) == "X=") {
      std::istringstream istr(tmp.c_str() + 2);
      istr >> data->x;
    } else if (tmp.substr(0, 2) == "R=") {
      std::istringstream istr(tmp.c_str() + 2);
      istr >> data->r;
    } else if (tmp.substr(0, 4) == "MSG=") {
      data->msg = tmp.c_str() + 4;
    } else if (tmp.substr(0, 3) == "MA=") {
      data->mawashi_area = tmp.c_str() + 3;
    }
  }
  data->status = OK;
}

int Connector::GetReaderFd() const {
  return reader_fd_;
}
