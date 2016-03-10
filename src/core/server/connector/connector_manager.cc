#include "core/server/connector/connector_manager.h"

#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/frame_response.h"
#include "core/server/connector/human_connector.h"
#include "core/server/connector/server_connector.h"
#include "core/player.h"

#if defined(_MSC_VER)
#include "core/server/connector/pipe_connector_win.h"
#else
#include "core/server/connector/pipe_connector_posix.h"
#endif

using namespace std;

ConnectorManager::ConnectorManager(bool timeout) :
    waitTimeout_(timeout)
{
}

void ConnectorManager::setConnector(int playerId, std::unique_ptr<ServerConnector> p)
{
    if (p->isHuman()) {
        humanConnectors_.push_back(static_cast<HumanConnector*>(p.get()));
    } else {
        pipeConnectors_.push_back(static_cast<PipeConnector*>(p.get()));
    }
    connectors_[playerId] = std::move(p);
}

bool ConnectorManager::receive(int frameId, vector<FrameResponse> cfr[NUM_PLAYERS])
{
    for (int i = 0; i < NUM_PLAYERS; ++i)
        cfr[i].clear();

    for (HumanConnector* ctr : humanConnectors_) {
        FrameResponse response;
        CHECK(ctr->receive(&response)) << "Human connector must be always receivable.";
        cfr[ctr->playerId()].push_back(response);
    }

#if defined(_MSC_VER)
    return PipeConnectorWin::pollAndReceive(waitTimeout_, frameId, pipeConnectors_, cfr);
#else
    return PipeConnectorPosix::pollAndReceive(waitTimeout_, frameId, pipeConnectors_, cfr);
#endif
}
