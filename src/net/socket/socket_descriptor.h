#ifndef NET_SOCKET_SOCKET_DESCRIPTOR_H_
#define NET_SOCKET_SOCKET_DESCRIPTOR_H_

#ifdef OS_WIN
# include <winsock2.h>
# undef ERROR
#endif

namespace net {

#ifdef OS_WIN
using SocketDescriptor = SOCKET;
// Windows has a definition of INVALID_SOCKET.
#else
using SocketDescriptor = int;
const SocketDescriptor INVALID_SOCKET = -1;
#endif

} // namespace net

#endif // NET_SOCKET_SOCKET_DESCRIPTOR_H_
