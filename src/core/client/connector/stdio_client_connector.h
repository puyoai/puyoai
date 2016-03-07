#ifndef CORE_CLIENT_CONNECTOR_STDIO_CLIENT_CONNECTOR_H_
#define CORE_CLIENT_CONNECTOR_STDIO_CLIENT_CONNECTOR_H_

#include "core/client/connector/client_connector.h"

struct FrameRequest;
struct FrameResponse;

class StdioClientConnector : public ClientConnector {
public:
    StdioClientConnector() {}

    bool isClosed() { return closed_; }

private:
    bool readExactly(void* buf, size_t size) override;
    bool writeExactly(const void* buf, size_t size) override;
    void flush() override;

    bool closed_ = false;
};

#endif // CORE_CLIENT_CONNECTOR_STDIO_CLIENT_CONNECTOR_H_
