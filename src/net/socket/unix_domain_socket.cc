#include "net/socket/unix_domain_socket.h"

#include <utility>

namespace net {

UnixDomainSocket::UnixDomainSocket(UnixDomainSocket&& socket) noexcept :
    Socket(std::move(socket))
{
}

UnixDomainSocket::~UnixDomainSocket()
{
}

UnixDomainSocket& UnixDomainSocket::operator=(UnixDomainSocket&& socket) noexcept
{
    std::swap(sd_, socket.sd_);
    return *this;
}

} // namespace net
