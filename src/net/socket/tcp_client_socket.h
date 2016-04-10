#ifndef NET_SOCKET_TCP_CLIENT_SOCKET_H_
#define NET_SOCKET_TCP_CLIENT_SOCKET_H_

#include "net/socket/tcp_socket.h"

namespace net {

class TCPClientSocket : public TCPSocket {
public:
    TCPClientSocket(TCPClientSocket&& socket);
    ~TCPClientSocket() override;

    bool connect(const char* host, int port);

private:
    explicit TCPClientSocket(SocketDescriptor sd) : TCPSocket(sd) {}

    friend class SocketFactory;
    DISALLOW_COPY_AND_ASSIGN(TCPClientSocket);
};

} // namespace net

#endif // NET_SOCKET_TCP_CLIENT_SOCKET_H_
