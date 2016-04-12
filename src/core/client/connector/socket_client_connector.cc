#include "core/client/connector/socket_client_connector.h"

#include <utility>

SocketClientConnector::SocketClientConnector(net::Socket socket) :
    socket_(std::move(socket))
{
}

bool SocketClientConnector::readExactly(void* buf, size_t size)
{
    return socket_.readExactly(buf, size);
}

bool SocketClientConnector::writeExactly(const void* buf, size_t size)
{
    return socket_.writeExactly(buf, size);
}

void SocketClientConnector::flush()
{
    // do nothing.
}
