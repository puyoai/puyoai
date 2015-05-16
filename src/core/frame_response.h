#ifndef CORE_FRAME_RESPONSE_H_
#define CORE_FRAME_RESPONSE_H_

#include <string>

#include "core/decision.h"
#include "core/key_set.h"

struct FrameResponse {
    static FrameResponse parse(const std::string&);

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
