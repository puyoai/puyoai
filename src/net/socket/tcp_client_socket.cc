#include "net/socket/tcp_client_socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "glog/logging.h"

namespace net {

TCPClientSocket::TCPClientSocket(TCPClientSocket&& socket) :
    TCPSocket(std::move(socket))
{
}

TCPClientSocket::~TCPClientSocket()
{
}

bool TCPClientSocket::connect(const char* host, int port)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = PF_INET;
    client_addr.sin_addr.s_addr = inet_addr(host);
    client_addr.sin_port = htons(port);

    if (::connect(sd_, reinterpret_cast<struct sockaddr*>(&client_addr), sizeof(client_addr)) < 0) {
        PLOG(ERROR) << "connect"
                    << " host=" << host
                    << " port=" << port;
        return false;
    }

    return true;
}

} // namespace net
