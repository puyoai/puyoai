#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_

#include <chrono>
#include <cstdio>
#include <string>

#include "core/server/connector/connector.h"

struct FrameRequest;
struct FrameResponse;

class PipeConnector : public Connector {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    static std::unique_ptr<Connector> create(int playerId, const std::string& program);
    static int getUsecFromStart(const TimePoint& start);
    static int getRemainingMilliSeconds(const TimePoint& start);

    virtual ~PipeConnector() override {}

    virtual void send(const FrameRequest&) override;
    virtual bool receive(FrameResponse*) override;

    virtual bool isHuman() const final { return false; }
    virtual bool isClosed() const final { return closed_; }
    void setClosed(bool flag) { closed_ = flag; }

protected:
    static const int kBufferSize = 1000;

    explicit PipeConnector(int player);

    virtual void writeString(const std::string&) = 0;
    virtual bool readString(char*) = 0;

private:
    bool closed_;
};

#endif // CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
