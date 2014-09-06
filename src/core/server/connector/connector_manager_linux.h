#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_LINUX_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_LINUX_H_

#include <memory>
#include <vector>

#include "base/base.h"
#include "core/server/connector/connector.h"
#include "core/server/connector/connector_manager.h"

class ConnectorManagerLinux : public ConnectorManager {
public:
    ConnectorManagerLinux(std::unique_ptr<Connector> p1, std::unique_ptr<Connector> p2);

    virtual void send(const ConnectorFrameRequest&) override;
    virtual bool receive(int frameId, std::vector<ConnectorFrameResponse> cfr[2]) override;

    virtual Connector* connector(int i) override { return connectors_[i].get(); }

    virtual void setWaitTimeout(bool flag) override { waitTimeout_ = flag; }

private:
    static const int NUM_PLAYERS = 2;
    std::unique_ptr<Connector> connectors_[NUM_PLAYERS];
    bool waitTimeout_;
};

#endif
