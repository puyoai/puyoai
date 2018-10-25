#ifndef NET_SOCKET_FACTORY_H_
#define NET_SOCKET_FACTORY_H_

#include "base/base.h"
#include "net/socket/tcp_socket.h"
#include "net/socket/unix_domain_client_socket.h"
#include "net/socket/unix_domain_server_socket.h"
#include "net/socket/tcp_client_socket.h"
#include "net/socket/tcp_server_socket.h"

namespace net {

class SocketFactory {
public:
    static SocketFactory* instance();

    // Returns TCP socket. If failed, socket will be invalid.
    TCPClientSocket makeTCPClientSocket();
    TCPServerSocket makeTCPServerSocket();

#ifdef OS_POSIX
    // Returns UnixDomainSocket. If failed, socket will be invalid.
    UnixDomainClientSocket makeUnixDomainClientSocket();
    UnixDomainServerSocket makeUnixDomainServerSocket();
#endif

private:
    SocketFactory() {}

    DISALLOW_COPY_AND_ASSIGN(SocketFactory);
};

} // namespace net

#endif // NET_SOCKET_FACTORY_H_
