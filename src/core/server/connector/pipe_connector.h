#ifndef CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_PIPE_CONNECTOR_H_

#include <cstdio>
#include <memory>

#include "core/server/connector/connector.h"

class PipeConnector : public Connector {
public:
    PipeConnector(int writerFd, int readerFd);
    virtual ~PipeConnector();

    virtual void write(const ConnectorFrameRequest&) override;
    virtual void writeString(const std::string&) override;
    virtual ConnectorFrameResponse read() override;
    virtual bool isHuman() const override { return false; }
    virtual bool alive() const override { return alive_; }
    virtual void setAlive(bool flag) override { alive_ = flag; }
    virtual bool pollable() const override { return true; }
    virtual int readerFd() const override { return readerFd_; }

private:
    bool alive_ = true;
    int writerFd_;
    int readerFd_;
    FILE* writer_;
    FILE* reader_;
};

#endif

