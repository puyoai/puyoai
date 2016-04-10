#include "net/socket/tcp_server_socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "glog/logging.h"

namespace net {

TCPServerSocket::TCPServerSocket(TCPServerSocket&& socket) :
    TCPSocket(std::move(socket))
{
}

TCPServerSocket::~TCPServerSocket()
{
}

bool TCPServerSocket::bindFromAny(int port)
{
    struct sockaddr_in reader_addr;
    memset(&reader_addr, 0, sizeof(reader_addr));

    reader_addr.sin_family = PF_INET;
    reader_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    reader_addr.sin_port = htons(port);

    if (::bind(sd_, reinterpret_cast<struct sockaddr*>(&reader_addr), sizeof(reader_addr)) < 0) {
        PLOG(ERROR) << "bind"
                    << " socket=" << sd_
                    << " port=" << port;
        return false;
    }

    return true;
}

bool TCPServerSocket::listen(int backlog)
{
    if (::listen(sd_, backlog) < 0) {
        PLOG(ERROR) << "listen"
                    << " socket=" << sd_
                    << " backlog=" << backlog;
        return false;
    }
    return true;
}

TCPSocket TCPServerSocket::accept()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int new_fd = ::accept(sd_, reinterpret_cast<struct sockaddr*>(&addr), &len);
    if (new_fd < 0) {
        PLOG(ERROR) << "accept" << " socket=" << sd_;
        return TCPSocket(INVALID_SOCKET);
    }

    return TCPSocket(new_fd);
}

} // namespace net
