#include "net/socket/unix_domain_socket.h"

#include <utility>

namespace net {

UnixDomainSocket::UnixDomainSocket(UnixDomainSocket&& socket) :
    Socket(std::move(socket))
{
}

UnixDomainSocket::~UnixDomainSocket()
{
}

} // namespace net
