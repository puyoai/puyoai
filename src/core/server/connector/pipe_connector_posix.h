#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_POSIX_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_POSIX_H_

#include <poll.h>

#include <cstdio>
#include <string>

#include "core/server/connector/pipe_connector.h"

struct FrameRequest;
struct FrameResponse;

class PipeConnectorPosix : public PipeConnector {
public:
    static std::unique_ptr<ServerConnector> create(int playerId, const std::string& program);

    virtual ~PipeConnectorPosix() override;

private:
    PipeConnectorPosix(int player, int writerFd, int readerFd);

    // Writes |size| bytes |data|.
    // Returns true if succeeded. False otherwise.
    bool writeData(const void* data, size_t size) override final;

    // Reads |size| bytes |data|.
    // Returns true if succeeded. False otherwise.
    bool readData(void* data, size_t size) override final;

    int writerFd_;
    int readerFd_;
};

#endif // CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_POSIX_H_
