#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_

#include <memory>
#include <vector>

#include "core/player.h"

class Connector;
class HumanConnector;
class PipeConnector;
struct FrameResponse;

class ConnectorManager {
public:
    // Receives decision and messages from clients.
    // Returns false when disconnected.
    ConnectorManager(std::unique_ptr<Connector> p1, std::unique_ptr<Connector> p2, bool timeout);

    bool receive(int frameId, std::vector<FrameResponse> cfr[NUM_PLAYERS]);

    Connector* connector(int i) { return connectors_[i].get(); }

private:
    std::unique_ptr<Connector> connectors_[NUM_PLAYERS];

    std::vector<HumanConnector*> humanConnectors_;
    std::vector<PipeConnector*> pipeConnectors_;

    bool waitTimeout_;
};

#endif // CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_
