#ifndef CORE_SERVER_CONNECTOR_HUMAN_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_HUMAN_CONNECTOR_H_

#include <mutex>
#include "core/server/connector/connector.h"

struct KeySet {
    bool downKey = false;
    bool leftKey = false;
    bool rightKey = false;
    bool rightTurnKey = false;
    bool leftTurnKey = false;
};

class HumanConnector : public Connector {
public:
    virtual ~HumanConnector() {}

    virtual void write(const std::string& message) OVERRIDE;
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
    KeySet currentKeySet_;
};

#endif
