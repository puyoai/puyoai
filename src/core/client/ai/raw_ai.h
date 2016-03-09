#ifndef CORE_CLIENT_RAW_AI_H_
#define CORE_CLIENT_RAW_AI_H_

#include <memory>

#include "core/client/connector/client_connector.h"

struct FrameRequest;
struct FrameResponse;

class RawAI {
public:
    RawAI();
    virtual ~RawAI() {}

    void runLoop();

protected:
    virtual FrameResponse playOneFrame(const FrameRequest&) = 0;

private:
    std::unique_ptr<ClientConnector> connector_;
};

#endif
