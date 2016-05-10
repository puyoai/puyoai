#ifndef CORE_SERVER_CONNECTOR_SERVER_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_SERVER_CONNECTOR_H_

#include <memory>
#include <string>
#include <vector>

#include "base/noncopyable.h"

struct FrameRequest;
struct FrameResponse;

class ServerConnector : noncopyable {
public:
    static std::unique_ptr<ServerConnector> create(int playerId, const std::string& program);

    explicit ServerConnector(int player) : playerId_(player) {}
    virtual ~ServerConnector() {}

    virtual void send(const FrameRequest&) = 0;
    virtual bool receive(FrameResponse*) = 0;

    virtual bool isHuman() const = 0;
    virtual bool isClosed() const = 0;

    int playerId() const { return playerId_; }

protected:
    static std::unique_ptr<ServerConnector> createStdioConnector(int playerId, const std::string& program);

#ifdef OS_POSIX
    static std::unique_ptr<ServerConnector> createTCPSocketConnector(int playerId, const std::string& program);
    static std::unique_ptr<ServerConnector> createTCPSocketConnectorFromPort(int playerId, int port);
#endif

    int playerId_;
};

#endif // CORE_SERVER_CONNECTOR_SERVER_CONNECTOR_H_
