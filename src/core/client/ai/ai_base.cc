#include "core/client/ai/ai_base.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/connector/socket_connector_impl.h"
#include "core/connector/stdio_connector_impl.h"
#if defined(USE_TCP)
#include "net/socket/socket_factory.h"
#include "net/socket/unix_domain_client_socket.h"
#endif

DEFINE_string(connector, "stdio", "Choose stdio,unix,tcp");
DEFINE_string(connector_socket_path, "/tmp/puyoai.sock", "path to unix domain socket");
DEFINE_int32(connector_port, 4321, "server port");

// static
std::unique_ptr<ClientConnector> AIBase::makeConnector()
{
    std::unique_ptr<ConnectorImpl> impl;

    if (FLAGS_connector == "stdio") {
        impl.reset(new StdioConnectorImpl());
#if defined(USE_TCP)
    } else if (FLAGS_connector == "unix") {
        net::UnixDomainClientSocket socket(net::SocketFactory::instance()->makeUnixDomainClientSocket());
        CHECK(socket.connect(FLAGS_connector_socket_path.c_str()));
        impl.reset(new SocketConnectorImpl(std::move(socket)));
#endif
    } else {
        CHECK(false) << "Unknown connector: " << FLAGS_connector;
    }

    return std::unique_ptr<ClientConnector>(new ClientConnector(std::move(impl)));
}
