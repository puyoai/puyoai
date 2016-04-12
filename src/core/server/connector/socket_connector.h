#ifndef CORE_SERVER_CONNECTOR_SOCKET_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_SOCKET_CONNECTOR_H_

#include "net/socket/socket.h"
#include "core/server/connector/pipe_connector.h"

class SocketConnector : public PipeConnector {
public:
    SocketConnector(int player_id, net::Socket socket);
    ~SocketConnector() override {}

    static std::unique_ptr<ServerConnector> create(int player_id, net::Socket socket);

protected:
    bool writeData(const void*, size_t) override;
    bool readData(void*, size_t) override;

    net::Socket socket_;
};

#endif // CORE_SERVER_CONNECTOR_SOCKET_CONNECTOR_H_
