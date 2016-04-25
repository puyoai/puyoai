#ifndef CORE_CONNECTOR_SOCKET_CONNECTOR_IMPL_H_
#define CORE_CONNECTOR_SOCKET_CONNECTOR_IMPL_H_

#include "core/connector/connector_impl.h"
#include "net/socket/socket.h"

class SocketConnectorImpl : public ConnectorImpl {
public:
    explicit SocketConnectorImpl(net::Socket socket);
    ~SocketConnectorImpl();

    bool readExactly(void* buf, size_t size) override;
    bool writeExactly(const void* buf, size_t size) override;
    void flush() override;

private:
    net::Socket socket_;
};

#endif // CORE_CONNECTOR_SOCKET_CONNECTOR_IMPL_H_
