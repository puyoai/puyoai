#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_H_

#include <cstdio>
#include <string>

#include "core/server/connector/game_log.h"

class Connector {
public:
    Connector();
    Connector(int writer_fd, int reader_fd);

    void Write(const std::string& message);
    ReceivedData Read();
    int GetReaderFd() const { return reader_fd_; }

private:
    ReceivedData parse(const char* str);

    int writer_fd_;
    int reader_fd_;
    FILE* writer_;
    FILE* reader_;
};

#endif
