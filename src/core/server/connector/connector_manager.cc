#include "core/server/connector/connector_manager.h"

#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/file/file.h"
#include "base/file/path.h"
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

using namespace std;

ConnectorManager::ConnectorManager(bool timeout) :
    waitTimeout_(timeout)
{
}

void ConnectorManager::invokePlayer(int playerId, const string& programName)
{
    if (programName == "-") {
        setConnector(playerId, unique_ptr<ServerConnector>(new HumanConnector(playerId)));
        return;
    }

    // Invoke player program. Player program should connector to /tmp/puyoai.sock.
    pid_t pid;
    {
        char* program_name = strdup(programName.c_str());
        char player_name[] = "Player_";
        player_name[6] = '1' + playerId;
        char* argv[3] = { program_name, player_name, nullptr };
        CHECK_EQ(posix_spawn(&pid, programName.c_str(), nullptr, nullptr, argv, nullptr), 0);

        free(program_name);
    }

    // Make server socket
    // TODO(mayah): Needs to unlink /tmp/puyoai.sock before binding.
    net::UnixDomainServerSocket socket = net::SocketFactory::instance()->makeUnixDomainServerSocket();
    file::remove("/tmp/puyoai.sock");
    socket.bind("/tmp/puyoai.sock");
    socket.listen(1);

    net::UnixDomainSocket accepted = socket.accept();
    setConnector(playerId, unique_ptr<ServerConnector>(new SocketConnector(playerId, std::move(accepted))));
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
