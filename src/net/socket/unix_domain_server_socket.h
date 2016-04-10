#ifndef NET_UNIX_DOMAIN_SERVER_SOCKET_H_
#define NET_UNIX_DOMAIN_SERVER_SOCKET_H_

#include "base/macros.h"
#include "net/socket/socket.h"
#include "net/socket/unix_domain_socket.h"

namespace net {

class UnixDomainServerSocket;
typedef UnixDomainServerSocket AcceptedUnixDomainServerSocket;

class UnixDomainServerSocket : public Socket {
public:
    UnixDomainServerSocket(UnixDomainServerSocket&& socket);
    ~UnixDomainServerSocket() override;

    bool bind(const char* path);
    bool listen(int backlog);

    UnixDomainSocket accept();

private:
    explicit UnixDomainServerSocket(SocketDescriptor sd) : Socket(sd) {}

    friend class SocketFactory;
    DISALLOW_COPY_AND_ASSIGN(UnixDomainServerSocket);
};

}

#endif // NET_UNIX_DOMAIN_SOCKET_H_
