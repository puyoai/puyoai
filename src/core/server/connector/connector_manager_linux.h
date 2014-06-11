#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_LINUX_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_LINUX_H_

#include <poll.h>
#include <vector>

#include "base/base.h"
#include "core/server/connector/connector.h"
#include "core/server/connector/connector_manager.h"

class ConnectorManagerLinux : public ConnectorManager {
public:
    ConnectorManagerLinux(std::vector<std::string> program_names);
    virtual void Write(int id, const std::string& message) OVERRIDE;
    virtual bool GetActions(int frame_id, std::vector<PlayerLog>* all_data) OVERRIDE;
    virtual bool IsConnectorAlive(int id) OVERRIDE;
    virtual std::string GetErrorLog() OVERRIDE;

    virtual Connector* connector(int i) OVERRIDE { return &connectors_[i]; }

    virtual void setWaitTimeout(bool flag) OVERRIDE { waitTimeout_ = flag; }

private:
    Connector CreateConnector(std::string program_name, int id);

    std::vector<Connector> connectors_;
    pollfd pollfds_[2];
    std::vector<bool> connector_is_alive_;
    bool waitTimeout_;
};

#endif
