#include "net/socket/socket.h"

#ifdef OS_POSIX
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/socket.h>
# include <sys/types.h>
#endif

#ifdef OS_WIN
# include <winsock2.h>
# undef ERROR
#endif

#include "glog/logging.h"

namespace net {

Socket::Socket(Socket&& socket) noexcept :
    sd_(socket.sd_)
{
    socket.sd_ = INVALID_SOCKET;
}

Socket::~Socket()
{
    if (!valid())
        return;

    LOG(INFO) << "socket " << sd_ << " is closing.";

#ifdef OS_WIN
    if (closesocket(sd_) < 0) {
        PLOG(ERROR) << "failed to close socket";
    }
#else
    if (close(sd_) < 0) {
        PLOG(ERROR) << "failed to close socket";
    }
#endif
}

Socket& Socket::operator=(Socket&& socket) noexcept
{
    std::swap(sd_, socket.sd_);
    return *this;
}

ssize_t Socket::read(void* buf, size_t size)
{
    // Windows uses char* for the second argument of recv(), so
    // we explictily cast it.
    // Other OSs cast char* back to void* implicitly, and it is OK.
    return ::recv(sd_, reinterpret_cast<char*>(buf), size, 0);
}

bool Socket::readExactly(void* buf, size_t size)
{
    while (size > 0) {
        ssize_t s = read(buf, size);
        if (s <= 0) {
            if (s == 0) {
                PLOG(ERROR) << "unexpected EOF";
                return false;
            }
            if (errno == EAGAIN)
                continue;

            PLOG(ERROR) << "failed to read";
            return false;
        }

        size -= s;
        buf = reinterpret_cast<char*>(buf) + s;
    }

    return true;
}

ssize_t Socket::write(const void* buf, size_t size)
{
    // Windows uses const char* for the second argument, so we cast it.
    // Other OSs cast it back to const void* implicitly.
    return ::send(sd_, reinterpret_cast<const char*>(buf), size, 0);
}

bool Socket::writeExactly(const void* buf, size_t size)
{
    while (size > 0) {
        ssize_t s = write(buf, size);
        if (s <= 0) {
            if (s == 0) {
                PLOG(ERROR) << "connection closed";
                return false;
            }
            if (errno == EAGAIN)
                continue;

            PLOG(ERROR) << "faied to write";
            return false;
        }

        size -= s;
        buf = reinterpret_cast<const char*>(buf) + s;
    }

    return true;
}

} // namespace net
