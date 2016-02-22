#ifndef CORE_FRAME_RESPONSE_H_
#define CORE_FRAME_RESPONSE_H_

#include <string>

#include "core/decision.h"
#include "core/key_set.h"

struct FrameResponseHeader {
    explicit FrameResponseHeader(uint32_t size = 0) : size(size) {}

    uint32_t size;
};

struct FrameResponse {
    static FrameResponse parsePayload(const char* payload, size_t size);

    FrameResponse() {}
    explicit FrameResponse(int frameId,
                           const Decision& decision = Decision(),
                           const std::string& message = std::string()) :
        frameId(frameId), decision(decision), message(message) {}

    // TODO(mayah): Rename this method.
    bool isValid() const;
    std::string toString() const;

    int frameId = -1;
    Decision decision;
    Decision preDecision;
    std::string message;

    // Mostly for HumanConnection.
    KeySet keySet;

    std::string mawashiArea;
};

#endif
