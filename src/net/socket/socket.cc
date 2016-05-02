#include "net/socket/socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

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

    if (close(sd_) < 0) {
        PLOG(ERROR) << "failed to close socket";
    }
}

Socket& Socket::operator=(Socket&& socket) noexcept
{
    std::swap(sd_, socket.sd_);
    return *this;
}

ssize_t Socket::read(void* buf, size_t size)
{
    return ::recv(sd_, buf, size, 0);
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
    return ::send(sd_, buf, size, 0);
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
