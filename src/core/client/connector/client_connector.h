#ifndef CORE_CLIENT_CONNECTOR_CLIENT_CONNECTOR_H_
#define CORE_CLIENT_CONNECTOR_CLIENT_CONNECTOR_H_

#include "base/macros.h"

struct FrameRequest;
struct FrameResponse;

class ClientConnector {
public:
    virtual ~ClientConnector();

    // Returns true if receive suceeded.
    bool receive(FrameRequest* request);
    void send(const FrameResponse&);

    bool isClosed() { return closed_; }

protected:
    ClientConnector() {}

    virtual bool readExactly(void* buf, size_t size) = 0;
    virtual bool writeExactly(const void* buf, size_t size) = 0;
    virtual void flush() = 0;

    bool closed_ = false;

    DISALLOW_COPY_AND_ASSIGN(ClientConnector);
};

#endif // CORE_CLIENT_CONNECTOR_CLIENT_CONNECTOR_H_
