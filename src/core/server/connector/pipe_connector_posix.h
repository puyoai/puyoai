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
    static std::unique_ptr<Connector> create(int playerId, const std::string& program);
    static bool pollAndReceive(bool waitTimeout, int frameId, const std::vector<PipeConnector*>& handles, std::vector<FrameResponse>* cfr);

    virtual ~PipeConnectorPosix() override;

private:
    PipeConnectorPosix(int player, int writerFd, int readerFd);

    virtual void writeString(const std::string&) final;
    virtual bool readString(char*) final;

    FILE* writer_;
    FILE* reader_;

    int writerFd_;
    int readerFd_;
};

#endif // CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_POSIX_H_
