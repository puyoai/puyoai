#ifndef NET_SOCKET_TCP_SERVER_SOCKET_H_
#define NET_SOCKET_TCP_SERVER_SOCKET_H_

#include "net/socket/tcp_socket.h"

namespace net {

class TCPServerSocket : public TCPSocket {
public:
    TCPServerSocket(TCPServerSocket&& socket);
    ~TCPServerSocket() override;

    bool bindFromAny(int port);
    bool listen(int backlog);

    TCPSocket accept();

private:
    explicit TCPServerSocket(SocketDescriptor sd) : TCPSocket(sd) {}

    friend class SocketFactory;
    DISALLOW_COPY_AND_ASSIGN(TCPServerSocket);
};

} // namespace net

#endif // NET_SOCKET_TCP_SERVER_SOCKET_H_
