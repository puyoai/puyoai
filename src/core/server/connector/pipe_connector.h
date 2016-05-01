#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_

#include <chrono>
#include <cstdio>
#include <string>

#include "core/server/connector/server_connector.h"

struct FrameRequest;
struct FrameResponse;

class PipeConnector : public ServerConnector {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    virtual ~PipeConnector() override {}

    virtual void send(const FrameRequest&) override;
    virtual bool receive(FrameResponse*) override;

    virtual bool isHuman() const final { return false; }
    virtual bool isClosed() const final { return closed_; }
    void setClosed(bool flag) { closed_ = flag; }

protected:
    static const int kBufferSize = 1024;

    explicit PipeConnector(int player);

    virtual bool writeData(const void*, size_t) = 0;
    virtual bool readData(void*, size_t) = 0;

private:
    bool closed_;
};

#endif // CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
