#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_H_

#include <cstdio>
#include <memory>
#include <string>

#include "base/base.h"
#include "core/server/connector/received_data.h"

class Connector : noncopyable {
public:
    static std::unique_ptr<Connector> create(int playerId, const std::string& program);

    virtual ~Connector() {}

    virtual void write(const std::string& message) = 0;
    virtual ReceivedData read() = 0;
    virtual bool isHuman() const = 0;

    virtual bool alive() const = 0;
    virtual void setAlive(bool) = 0;

    // Returns true if this connector can use poll().
    virtual bool pollable() const = 0;
    // Returns reader file descriptor. Valid only when pollable() == true.
    virtual int readerFd() const = 0;

protected:
    ReceivedData parse(const char* str);
};

class PipeConnector : public Connector {
public:
    PipeConnector(int writerFd, int readerFd);
    virtual ~PipeConnector();

    virtual void write(const std::string& message) OVERRIDE;
    virtual ReceivedData read() OVERRIDE;
    virtual bool isHuman() const OVERRIDE { return false; }
    virtual bool alive() const OVERRIDE { return alive_; }
    virtual void setAlive(bool flag) OVERRIDE { alive_ = flag; }
    virtual bool pollable() const OVERRIDE { return true; }
    virtual int readerFd() const OVERRIDE { return readerFd_; }

private:
    bool alive_ = true;
    int writerFd_;
    int readerFd_;
    FILE* writer_;
    FILE* reader_;
};

class HumanConnector : public Connector {
public:
    virtual ~HumanConnector() {}

    virtual void write(const std::string& message) OVERRIDE;
    virtual ReceivedData read() OVERRIDE;
    virtual bool isHuman() const OVERRIDE { return true; }
    // HumanConnector is always alive.
    virtual bool alive() const OVERRIDE { return true; }
    virtual void setAlive(bool flag) OVERRIDE;
    virtual bool pollable() const OVERRIDE { return false; }
    virtual int readerFd() const OVERRIDE;
};

#endif
