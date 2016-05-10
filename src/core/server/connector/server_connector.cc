#include "core/server/connector/server_connector.h"

#include <string>

#include <glog/logging.h>

#include "base/memory.h"
#include "base/strings.h"
#include "core/server/connector/human_connector.h"

// TODO(mayah): These should not be POSIX only. Implement this for Win, and
// allow windows users to use these.
#ifdef OS_POSIX
# include "core/server/connector/socket_connector.h"
# include "net/socket/tcp_server_socket.h"
# include "net/socket/socket_factory.h"
#endif

#ifdef OS_WIN
# include "core/server/connector/pipe_connector_win.h"
#else
# include "core/server/connector/pipe_connector_posix.h"
#endif

DEFINE_string(server_connector, "stdio", "set connector type: stdio, unix, or tcp");

using namespace std;

// static
unique_ptr<ServerConnector> ServerConnector::create(int playerId, const string& programName)
{
    CHECK(0 <= playerId && playerId < 10) << playerId;

    // When programName is special, we do special handling.
    // Otherwise, we follow server_connector option.

    if (programName == "-")
        return unique_ptr<ServerConnector>(new HumanConnector(playerId));

#ifdef OS_POSIX
    if (strings::hasPrefix(programName, "tcp:")) {
        std::string port_str = programName.substr(4);
        CHECK(strings::isAllDigits(port_str)) << port_str;
        int port = atoi(port_str.c_str());

        return createTCPSocketConnectorFromPort(playerId, port);
    }
#endif

    if (FLAGS_server_connector == "stdio")
        return createStdioConnector(playerId, programName);

#ifdef OS_POSIX
    if (FLAGS_server_connector == "tcp")
        return createTCPSocketConnector(playerId, programName);
#endif

    CHECK(false) << "Unknown connector";
}

unique_ptr<ServerConnector> ServerConnector::createStdioConnector(int playerId, const string& programName)
{
#ifdef OS_WIN
    return PipeConnectorWin::create(playerId, programName);
#else
    return PipeConnectorPosix::create(playerId, programName);
#endif
}

#ifdef OS_POSIX
unique_ptr<ServerConnector> ServerConnector::createTCPSocketConnector(int playerId, const string& programName)
{
    // Invoke player program with TCP socket.
    base::unique_ptr_malloc<char> program_name(strdup(programName.c_str()));
    char player_name[] = "Player_";
    player_name[6] = '1' + playerId;

    char connector[] = "--connector=tcp:127.0.0.1:24241";
    connector[strlen(connector) - 1] = '1' + playerId;

    pid_t pid = fork();
    if (pid < 0) {
        PLOG(FATAL) << "failed to fork";
        return unique_ptr<ServerConnector>();
    }

    if (pid == 0) {
        // child
        if (execl(program_name.get(), program_name.get(), player_name, connector, nullptr) < 0) {
            PLOG(FATAL) << "failed to exec";
            return unique_ptr<ServerConnector>();
        }

        PLOG(FATAL) << "should not be reached here";
    }

    return createTCPSocketConnectorFromPort(playerId, playerId == 0 ? 24241 : 24242);
}

unique_ptr<ServerConnector> ServerConnector::createTCPSocketConnectorFromPort(int playerId, int port)
{
    // Make server socket.
    net::TCPServerSocket socket = net::SocketFactory::instance()->makeTCPServerSocket();
    socket.bindFromAny(port);
    socket.listen();

    net::TCPSocket accepted = socket.accept();
    CHECK(accepted.valid());

    LOG(INFO) << "tcp: accepted (player 1) sd=" << accepted.get();

    accepted.setTCPNodelay();

    return unique_ptr<ServerConnector>(new SocketConnector(playerId, std::move(accepted)));
}

#endif
