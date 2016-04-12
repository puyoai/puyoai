#include "core/server/connector/socket_connector.h"

SocketConnector::SocketConnector(int player_id, net::Socket socket) :
    PipeConnector(player_id),
    socket_(std::move(socket))
{
}

bool SocketConnector::writeData(const void* data, size_t size)
{
    return socket_.writeExactly(data, size);
}

bool SocketConnector::readData(void* data, size_t size)
{
    return socket_.readExactly(data, size);
}
