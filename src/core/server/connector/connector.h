#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_H_

#include <memory>
#include <string>
#include <vector>

#include "base/noncopyable.h"

struct FrameRequest;
struct FrameResponse;

class Connector : noncopyable {
public:
    static std::unique_ptr<Connector> create(int playerId, const std::string& program);

    explicit Connector(int player) : playerId_(player) {}
    virtual ~Connector() {}

    virtual void send(const FrameRequest&) = 0;
    virtual bool receive(FrameResponse*) = 0;

    virtual bool isHuman() const = 0;
    virtual bool isClosed() const = 0;

    int playerId() const { return playerId_; }

protected:
    int playerId_;
};

#endif // CORE_SERVER_CONNECTOR_CONNECTOR_H_
