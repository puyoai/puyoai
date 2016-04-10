#include "net/socket/unix_domain_server_socket.h"

#include <glog/logging.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <utility>

namespace net {

UnixDomainServerSocket::UnixDomainServerSocket(UnixDomainServerSocket&& socket) :
    Socket(std::move(socket))
{
}

UnixDomainServerSocket::~UnixDomainServerSocket()
{
}

bool UnixDomainServerSocket::bind(const char* path)
{
    DCHECK(valid());

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, 108);

    if (::bind(sd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        PLOG(ERROR) << "failed to bind"
                    << " sd=" << sd_
                    << " path=" << path;
        return false;
    }

    return true;
}

bool UnixDomainServerSocket::listen(int backlog)
{
    if (::listen(sd_, backlog) < 0) {
        PLOG(ERROR) << "listen"
                    << " sd=" << sd_
                    << " backlog=" << backlog;
        return false;
    }

    return true;
}

UnixDomainSocket UnixDomainServerSocket::accept()
{
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addr_len = sizeof(addr);

    int fd = ::accept(sd_, reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
    if (fd < 0) {
        PLOG(ERROR) << "failed to accept";
        return UnixDomainSocket(INVALID_SOCKET);
    }

    return UnixDomainSocket(fd);
}

} // namespace net
