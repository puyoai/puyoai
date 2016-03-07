#ifndef CORE_CLIENT_RAW_AI_H_
#define CORE_CLIENT_RAW_AI_H_

#include "core/client/connector/stdio_client_connector.h"

struct FrameRequest;
struct FrameResponse;

class RawAI {
public:
    virtual ~RawAI() {}

    void runLoop();

protected:
    virtual FrameResponse playOneFrame(const FrameRequest&) = 0;

private:
    StdioClientConnector connector_;
};

#endif
