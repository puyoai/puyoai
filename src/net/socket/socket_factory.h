#ifndef NET_SOCKET_FACTORY_H_
#define NET_SOCKET_FACTORY_H_

#include "base/macros.h"
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
    TCPClientSocket make_tcp_client_socket();
    TCPServerSocket make_tcp_server_socket();

    // Returns UnixDomainSocket. If failed, socket will be invalid.
    UnixDomainClientSocket makeUnixDomainClientSocket();
    UnixDomainServerSocket makeUnixDomainServerSocket();

private:
    SocketFactory() {}

    DISALLOW_COPY_AND_ASSIGN(SocketFactory);
};

} // namespace net

#endif // NET_SOCKET_FACTORY_H_
