#include "net/socket/socket_factory.h"

#include <sys/socket.h>

#include "glog/logging.h"

namespace net {

SocketFactory* SocketFactory::instance()
{
    static SocketFactory instance;
    return &instance;
}

TCPClientSocket SocketFactory::make_tcp_client_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        PLOG(ERROR) << "failed to make socket";
        return TCPClientSocket(INVALID_SOCKET);
    }

    return TCPClientSocket(sock);
}

TCPServerSocket SocketFactory::make_tcp_server_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        PLOG(ERROR) << "failed to make socket";
        return TCPServerSocket(INVALID_SOCKET);
    }

    return TCPServerSocket(sock);
}

UnixDomainClientSocket SocketFactory::makeUnixDomainClientSocket()
{
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        PLOG(ERROR) << "failed to make socket";
        return UnixDomainClientSocket(INVALID_SOCKET);
    }

    return UnixDomainClientSocket(sock);
}

UnixDomainServerSocket SocketFactory::makeUnixDomainServerSocket()
{
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        PLOG(ERROR) << "failed to make socket";
        return UnixDomainServerSocket(INVALID_SOCKET);
    }

    return UnixDomainServerSocket(sock);
}

} // namespace net
