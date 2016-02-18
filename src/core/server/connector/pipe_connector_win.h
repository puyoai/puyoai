#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_WIN_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_WIN_H_

#define NOMINMAX
#include <windows.h>

#include <cstdio>
#include <string>

#include "core/server/connector/pipe_connector.h"

struct FrameRequest;
struct FrameResponse;

class PipeConnectorWin : public PipeConnector {
public:
    static std::unique_ptr<Connector> create(int playerId, const std::string& program);
    static bool pollAndReceive(bool waitTimeout, int frameId, const std::vector<PipeConnector*>& pipeConnectors, std::vector<FrameResponse>* cfr);

    virtual ~PipeConnectorWin() override;

private:
    PipeConnectorWin(int player, HANDLE writer, HANDLE reader);

    virtual void writeString(const std::string&) final;
    virtual bool readString(char*) final;

    HANDLE writer_;
    HANDLE reader_;
};

#endif // CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_WIN_H_
