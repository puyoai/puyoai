#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_H_

#include <memory>
#include <string>

#include "base/base.h"
#include "core/server/connector/connector_frame_response.h"

struct FrameRequest;

class Connector : noncopyable {
public:
    static std::unique_ptr<Connector> create(int playerId, const std::string& program);

    virtual ~Connector() {}

    virtual void write(const FrameRequest&) = 0;
    virtual ConnectorFrameResponse read() = 0;
    virtual bool isHuman() const = 0;

    virtual bool alive() const = 0;
    virtual void setAlive(bool) = 0;

    // Returns true if this connector can use poll().
    virtual bool pollable() const = 0;
    // Returns reader file descriptor. Valid only when pollable() == true.
    virtual int readerFd() const = 0;
};

#endif
