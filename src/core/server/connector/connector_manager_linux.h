#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_LINUX_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_LINUX_H_

#include <vector>

#include "base/base.h"
#include "core/server/connector/connector.h"
#include "core/server/connector/connector_manager.h"

class ConnectorManagerLinux : public ConnectorManager {
public:
    ConnectorManagerLinux(std::vector<std::string> program_names);
    virtual void Write(int id, const std::string& message) OVERRIDE;
    virtual bool GetActions(int frame_id, std::vector<PlayerLog>* all_data) OVERRIDE;
    virtual std::string GetErrorLog() OVERRIDE;

    virtual Connector* connector(int i) OVERRIDE { return connectors_[i].get(); }

    virtual void setWaitTimeout(bool flag) OVERRIDE { waitTimeout_ = flag; }

private:
    std::vector<std::unique_ptr<Connector>> connectors_;
    bool waitTimeout_;
};

#endif
