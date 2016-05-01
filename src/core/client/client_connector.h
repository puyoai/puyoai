#ifndef CORE_CLIENT_CONNECTOR_CLIENT_CONNECTOR_H_
#define CORE_CLIENT_CONNECTOR_CLIENT_CONNECTOR_H_

#include <memory>

#include "base/base.h"
#include "core/connector/connector_impl.h"

struct FrameRequest;
struct FrameResponse;

class ClientConnector {
public:
    explicit ClientConnector(std::unique_ptr<ConnectorImpl> impl) : impl_(std::move(impl)) {}
    ~ClientConnector();

    // Returns true if receive suceeded.
    bool receive(FrameRequest* request);
    void send(const FrameResponse&);

    bool isClosed() { return closed_; }

protected:
    bool closed_ = false;
    std::unique_ptr<ConnectorImpl> impl_;

    DISALLOW_COPY_AND_ASSIGN(ClientConnector);
};

#endif // CORE_CLIENT_CONNECTOR_CLIENT_CONNECTOR_H_
