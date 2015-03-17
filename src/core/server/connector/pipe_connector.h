#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_

#include <cstdio>
#include <string>

#include "core/server/connector/connector.h"

struct FrameRequest;
struct FrameResponse;

class PipeConnector : public Connector {
public:
    PipeConnector(int writerFd, int readerFd);
    virtual ~PipeConnector();

    virtual void send(const FrameRequest&) override;
    virtual bool receive(FrameResponse*) override;

    virtual bool isHuman() const override { return false; }
    virtual bool isClosed() const override { return closed_; }
    virtual void setClosed(bool flag) override { closed_ = flag; }
    virtual bool pollable() const override { return true; }
    virtual int readerFd() const override { return readerFd_; }

private:
    void writeString(const std::string&);

    bool closed_ = false;

    int writerFd_;
    int readerFd_;
    FILE* writer_;
    FILE* reader_;
};

#endif

