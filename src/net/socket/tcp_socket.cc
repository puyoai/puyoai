#include "net/socket/tcp_socket.h"

#ifdef OS_WIN
# include <winsock2.h>
# undef ERROR
#else
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/socket.h>
# include <sys/types.h>
#endif

#include "glog/logging.h"

namespace net {

TCPSocket::TCPSocket(TCPSocket&& socket) noexcept :
    Socket(std::move(socket))
{
}

TCPSocket::~TCPSocket()
{
}

TCPSocket& TCPSocket::operator=(TCPSocket&& socket) noexcept
{
    std::swap(sd_, socket.sd_);
    return *this;
}

bool TCPSocket::setTCPNodelay()
{
    int flag = 1;
#ifdef OS_WIN
    const char* optval = reinterpret_cast<const char*>(&flag);
#else
    int* optval = &flag;
#endif
    if (setsockopt(sd_, IPPROTO_TCP, TCP_NODELAY, optval, sizeof(flag)) < 0) {
        PLOG(ERROR) << "failed to set TCP_NODELAY";
        return false;
    }

    return true;
}

} // namespace net
