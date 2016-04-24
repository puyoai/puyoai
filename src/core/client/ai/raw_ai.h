#ifndef CORE_CLIENT_RAW_AI_H_
#define CORE_CLIENT_RAW_AI_H_

#include <memory>

#include "core/client/ai/ai_base.h"
#include "core/client/client_connector.h"

struct FrameRequest;
struct FrameResponse;

class RawAI : public AIBase {
public:
    RawAI();
    virtual ~RawAI() override {}

    void runLoop();

protected:
    virtual FrameResponse playOneFrame(const FrameRequest&) = 0;

private:
    std::unique_ptr<ClientConnector> connector_;
};

#endif
