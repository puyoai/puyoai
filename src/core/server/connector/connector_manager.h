#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_

#include <string>
#include <vector>

#include "core/server/connector/received_data.h"

class Connector;

class ConnectorManager {
public:
    virtual bool GetActions(int frame_id, std::vector<PlayerLog>* all_data) = 0;

    virtual Connector* connector(int i) = 0;

    virtual void setWaitTimeout(bool) = 0;
};

#endif
