#include "net/socket/tcp_socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "glog/logging.h"

namespace net {

TCPSocket::TCPSocket(TCPSocket&& socket) :
    Socket(std::move(socket))
{
}

TCPSocket::~TCPSocket()
{
}

bool TCPSocket::set_tcpnodelay()
{
    int flag = 1;
    if (setsockopt(sd_, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        PLOG(ERROR) << "failed to set TCP_NODELAY";
        return false;
    }

    return true;
}

} // namespace net
