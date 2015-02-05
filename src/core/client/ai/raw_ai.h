#ifndef CORE_CLIENT_RAW_AI_H_
#define CORE_CLIENT_RAW_AI_H_

#include "core/client/connector/client_connector.h"

class RawAI {
public:
    virtual ~RawAI() {}

    void runLoop();

protected:
    virtual FrameResponse playOneFrame(const FrameRequest&) = 0;

private:
    ClientConnector connector_;
};

#endif
