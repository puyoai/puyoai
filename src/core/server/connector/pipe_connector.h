#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_

#include <cstdio>
#include <string>

#include "core/server/connector/connector.h"

struct FrameRequest;
struct FrameResponse;

class PipeConnector : public Connector {
public:
    static std::unique_ptr<Connector> create(int playerId, const std::string& program);

    virtual ~PipeConnector() override {}

    virtual void send(const FrameRequest&) override;
    virtual bool receive(FrameResponse*) override;

    virtual bool isHuman() const final { return false; }
    virtual bool isClosed() const final { return closed_; }

protected:
    static const int kBufferSize = 1000;

    explicit PipeConnector(int player);

    void setClosed(bool flag) { closed_ = flag; }

    virtual void writeString(const std::string&) = 0;
    virtual bool readString(char*) = 0;

private:
    bool closed_;
};

#endif // CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
