#include "core/client/ai/ai_base.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/strings.h"
#include "core/connector/socket_connector_impl.h"
#include "core/connector/stdio_connector_impl.h"
#if defined(USE_TCP)
#include "net/socket/socket_factory.h"
#include "net/socket/unix_domain_client_socket.h"
#endif

DEFINE_string(connector, "stdio", "stdio, unix:<unix domain path>, or tcp:<hostname>:<port>");

// static
std::unique_ptr<ClientConnector> AIBase::makeConnector()
{
    if (FLAGS_connector == "stdio") {
        std::unique_ptr<ConnectorImpl> impl(new StdioConnectorImpl());
        return std::unique_ptr<ClientConnector>(new ClientConnector(std::move(impl)));
    }

#if defined(USE_TCP) && defined(OS_POSIX)
    if (strings::hasPrefix(FLAGS_connector, "unix:")) {
        net::UnixDomainClientSocket socket(net::SocketFactory::instance()->makeUnixDomainClientSocket());

        std::string path = FLAGS_connector.substr(5);
        CHECK(socket.connect(path.c_str()));
        std::unique_ptr<ConnectorImpl> impl(new SocketConnectorImpl(std::move(socket)));
        return std::unique_ptr<ClientConnector>(new ClientConnector(std::move(impl)));
    }
#endif

#if defined(USE_TCP) && defined(OS_POSIX)
    if (strings::hasPrefix(FLAGS_connector, "tcp:")) {
        std::string host_port = FLAGS_connector.substr(4);
        std::vector<std::string> parts = strings::split(host_port, ':');
        CHECK_EQ(parts.size(), 2U) << "tcp:<hostname>:<port> is expected, but " << FLAGS_connector;

        std::string host = parts[0];
        std::string port_str = parts[1];
        CHECK(!port_str.empty() && !host.empty()) << "tcp:<hostname>:<port> is expected, but " << FLAGS_connector;
        CHECK(strings::isAllDigits(port_str)) << "tcp:<hostname>:<port> is expected, but " << FLAGS_connector;

        int port = atoi(port_str.c_str());

        net::TCPClientSocket socket(net::SocketFactory::instance()->makeTCPClientSocket());
        CHECK(socket.setTCPNodelay());
        CHECK(socket.connect(host.c_str(), port));

        std::unique_ptr<ConnectorImpl> impl(new SocketConnectorImpl(std::move(socket)));
        return std::unique_ptr<ClientConnector>(new ClientConnector(std::move(impl)));
    }
#endif

    CHECK(false) << "Unknown connector: " << FLAGS_connector;
}
