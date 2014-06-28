#ifndef CORE_SERVER_CONNECTOR_HUMAN_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_HUMAN_CONNECTOR_H_

#include <mutex>
#include "core/server/connector/connector.h"
#include "core/key_set.h"

class HumanConnector : public Connector {
public:
    explicit HumanConnector(int playerId) : Connector(playerId) {}
    virtual ~HumanConnector() {}

    virtual void write(const ConnectorFrameRequest&) OVERRIDE;
    virtual void writeString(const std::string&) OVERRIDE;
    virtual ConnectorFrameResponse read() OVERRIDE;
    virtual bool isHuman() const OVERRIDE { return true; }
    // HumanConnector is always alive.
    virtual bool alive() const OVERRIDE { return true; }
    virtual void setAlive(bool flag) OVERRIDE;
    virtual bool pollable() const OVERRIDE { return false; }
    virtual int readerFd() const OVERRIDE;

    void setKeySet(const KeySet&);

private:
    std::mutex mu_;
    bool nextIsPlayable_ = false;
    KeySet currentKeySet_;
};

#endif
