#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_WIN_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_WIN_H_

#include <windows.h>

#include <cstdio>
#include <string>

#include "core/server/connector/pipe_connector.h"

struct FrameRequest;
struct FrameResponse;

class PipeConnectorWin : public PipeConnector {
public:
    static std::unique_ptr<ServerConnector> create(int playerId, const std::string& program);

    virtual ~PipeConnectorWin() override;

private:
    PipeConnectorWin(int player, HANDLE writer, HANDLE reader);

    bool writeData(const void* data, size_t size) override final;
    bool readData(void* data, size_t size) override final;

    HANDLE writer_;
    HANDLE reader_;
};

#endif // CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_WIN_H_
