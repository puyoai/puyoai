#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "base/blocking_queue.h"
#include "core/frame_response.h"
#include "core/player.h"

class HumanConnector;
class PipeConnector;
class ServerConnector;
struct FrameResponse;

class ConnectorManager {
public:
    explicit ConnectorManager(bool always_wait_timeout);
    ~ConnectorManager();

    bool receive(int frameId, std::vector<FrameResponse> cfr[NUM_PLAYERS],
                 const std::chrono::steady_clock::time_point& timeout_time);

    void setPlayer(int player_id, const std::string& program);

    // Starts receiver threads.
    void start();
    // Stops receiver threads.
    void stop();

    ServerConnector* connector(int i) { return connectors_[i].get(); }

private:
    static void receiverThreadDriver(ConnectorManager* manager, int player_id);
    void runReceiverThreadLoop(int player_id);

    void setConnector(int playerId, std::unique_ptr<ServerConnector> p);

    std::unique_ptr<ServerConnector> connectors_[NUM_PLAYERS];

    std::vector<HumanConnector*> humanConnectors_;
    std::vector<PipeConnector*> pipeConnectors_;

    std::atomic<bool> should_stop_;

    // If true, ConnectorManager always consume 16ms.
    bool always_wait_timeout_;
    base::InfiniteBlockingQueue<FrameResponse> resp_queue_[2];
    std::thread receiver_thread_[2];
};

#endif // CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_
