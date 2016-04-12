#ifndef CORE_CLIENT_CONNECTOR_SOCKET_CLIENT_CONNECTOR_H_
#define CORE_CLIENT_CONNECTOR_SOCKET_CLIENT_CONNECTOR_H_

#include "core/client/connector/client_connector.h"
#include "net/socket/socket.h"

struct FrameRequest;
struct FrameResponse;

class SocketClientConnector : public ClientConnector {
public:
    explicit SocketClientConnector(net::Socket socket);

private:
    bool readExactly(void* buf, size_t size) override;
    bool writeExactly(const void* buf, size_t size) override;
    void flush() override;

    net::Socket socket_;
};

#endif // CORE_CLIENT_CONNECTOR_SOCKET_CLIENT_CONNECTOR_H_
