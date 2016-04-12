#include "core/client/ai/ai_base.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/client/connector/socket_client_connector.h"
#include "core/client/connector/stdio_client_connector.h"
#include "net/socket/socket_factory.h"
#include "net/socket/unix_domain_client_socket.h"

DEFINE_string(connector, "stdio", "Choose stdio,unix,tcp");
DEFINE_string(connector_socket_path, "/tmp/puyoai.sock", "path to unix domain socket");
DEFINE_int32(connector_port, 4321, "server port");

// static
std::unique_ptr<ClientConnector> AIBase::makeConnector()
{
    if (FLAGS_connector == "stdio") {
        return std::unique_ptr<ClientConnector>(new StdioClientConnector);
    } else if (FLAGS_connector == "unix") {
        net::UnixDomainClientSocket socket(net::SocketFactory::instance()->makeUnixDomainClientSocket());
        CHECK(socket.connect(FLAGS_connector_socket_path.c_str()));
        return std::unique_ptr<ClientConnector>(new SocketClientConnector(std::move(socket)));
    } else {
        CHECK(false) << "Unknown connector: " << FLAGS_connector;
        return std::unique_ptr<ClientConnector>();
    }
}
