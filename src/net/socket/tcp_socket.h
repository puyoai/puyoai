#ifndef NET_SOCKET_TCP_SOCKET_H_
#define NET_SOCKET_TCP_SOCKET_H_

#include "net/socket/socket.h"

namespace net {

class TCPSocket : public Socket {
public:
    TCPSocket(TCPSocket&& socket);
    ~TCPSocket() override;

    bool set_tcpnodelay();

protected:
    explicit TCPSocket(SocketDescriptor sd) : Socket(sd) {}

    friend class TCPServerSocket;
    DISALLOW_COPY_AND_ASSIGN(TCPSocket);
};

} // namespace net

#endif // NET_SOCKET_TCP_SOCKET_H_
