#include "core/server/connector/connector_manager.h"

#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/frame_response.h"
#include "core/server/connector/connector.h"
#include "core/server/connector/human_connector.h"
#include "core/player.h"

#if defined(_MSC_VER)
#include "core/server/connector/pipe_connector_win.h"
#else
#include "core/server/connector/pipe_connector_posix.h"
#endif

using namespace std;

DEFINE_bool(realtime, true, "use realtime");
DEFINE_bool(no_timeout, false, "if true, wait ai's thought without timeout");

ConnectorManager::ConnectorManager(unique_ptr<Connector> p1, unique_ptr<Connector> p2, bool timeout) :
    connectors_{ move(p1), move(p2) },
    waitTimeout_(timeout)
{
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        Connector* ctr = connector(i);
        if (ctr->isHuman()) {
            humanConnectors_.push_back(static_cast<HumanConnector*>(ctr));
        } else {
            pipeConnectors_.push_back(static_cast<PipeConnector*>(ctr));
        }
    }
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

