#include "core/server/connector/connector_manager.h"

#include <chrono>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "base/file/file.h"
#include "base/file/path.h"
#include "core/frame.h"
#include "core/frame_response.h"
#include "core/server/connector/human_connector.h"
#include "core/server/connector/server_connector.h"
#include "core/player.h"
#include "net/socket/socket_factory.h"

#ifdef OS_WIN
#include "core/server/connector/pipe_connector_win.h"
#else
#include <spawn.h>
#include "core/server/connector/pipe_connector_posix.h"
#include "core/server/connector/socket_connector.h"
#include "net/socket/unix_domain_socket.h"
#endif

DEFINE_bool(realtime, true, "use realtime");
DEFINE_bool(timeout, true, "if false, wait ai's thought without timeout");

using namespace std;

ConnectorManager::ConnectorManager(bool always_wait_timeout) :
    should_stop_(false),
    always_wait_timeout_(always_wait_timeout)
{
}

ConnectorManager::~ConnectorManager()
{
}

void ConnectorManager::start()
{
    for (int i = 0; i < 2; ++i) {
        if (!connectors_[i]->isHuman())
            receiver_thread_[i] = std::thread(receiverThreadDriver, this, i);
    }
}

void ConnectorManager::stop()
{
    should_stop_ = true;
    for (int i = 0; i < 2; ++i) {
        if (receiver_thread_[i].joinable()) {
            receiver_thread_[i].join();
        }
    }
}

// static
void ConnectorManager::receiverThreadDriver(ConnectorManager* manager, int player_id)
{
    manager->runReceiverThreadLoop(player_id);
}

void ConnectorManager::runReceiverThreadLoop(int player_id)
{
    while (!should_stop_) {
        FrameResponse resp;
        if (!connectors_[player_id]->receive(&resp)) {
            LOG(INFO) << "failed to receive";
            return;
        }

        resp_queue_[player_id].push(resp);
    }
}

void ConnectorManager::setPlayer(int player_id, const std::string& program)
{
    setConnector(player_id, ServerConnector::create(player_id, program));
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

bool ConnectorManager::receive(int frameId, vector<FrameResponse> cfr[NUM_PLAYERS],
                               const std::chrono::steady_clock::time_point& timeout_time)
{
    for (int i = 0; i < NUM_PLAYERS; ++i)
        cfr[i].clear();

    for (HumanConnector* ctr : humanConnectors_) {
        FrameResponse response;
        CHECK(ctr->receive(&response)) << "Human connector must be always receivable.";
        cfr[ctr->playerId()].push_back(response);
    }

    // We have 16 milliseconds margin.
    auto real_timeout = timeout_time;
    auto timeout = FLAGS_timeout ? real_timeout : std::chrono::steady_clock::time_point::max();

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (connectors_[i]->isHuman())
            continue;

        std::vector<FrameResponse> resps;
        FrameResponse resp;

        while (resp_queue_[i].takeWithTimeout(timeout, &resp)) {
            resps.push_back(resp);
            if (resp.frameId == frameId)
                break;
        }

        // timeout or desired frame response is retrieved.
        cfr[i] = std::move(resps);
    }

    // All data is collected (or timeout) here.
    if (FLAGS_realtime || always_wait_timeout_) {
        // Needs to wait unilt real_timeout
        auto now = std::chrono::steady_clock::now();
        if (now < real_timeout) {
            auto d = real_timeout - now;
            std::this_thread::sleep_for(d);
        }
    }

    return true;
}
