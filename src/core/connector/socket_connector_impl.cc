#include "core/connector/socket_connector_impl.h"

#include <utility>

SocketConnectorImpl::SocketConnectorImpl(net::Socket socket) :
    socket_(std::move(socket))
{
}

SocketConnectorImpl::~SocketConnectorImpl()
{
}

bool SocketConnectorImpl::readExactly(void* buf, size_t size)
{
    return socket_.readExactly(buf, size);
}

bool SocketConnectorImpl::writeExactly(const void* buf, size_t size)
{
    return socket_.writeExactly(buf, size);
}

void SocketConnectorImpl::flush()
{
    // do nothing.
}
