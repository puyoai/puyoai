#ifndef CORE_CLIENT_CONNECTOR_CLIENT_FRAME_RESPONSE_H_
#define CORE_CLIENT_CONNECTOR_CLIENT_FRAME_RESPONSE_H_

#include <string>

#include "core/decision.h"

class ClientFrameResponse {
public:
    explicit ClientFrameResponse(int frameId);
    ClientFrameResponse(int frameId, const Decision& decision, const std::string& message = std::string());

    int frameId() const { return frameId_; }
    const Decision& decision() const { return decision_; }
    const std::string& message() const { return message_; }

private:
    int frameId_;
    Decision decision_;
    std::string message_;
};

#endif
