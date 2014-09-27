#ifndef CORE_FRAME_RESPONSE_H_
#define CORE_FRAME_RESPONSE_H_

#include <string>

#include "core/decision.h"
#include "core/key_set.h"

struct FrameResponse {
    static FrameResponse parse(const char* str);

    FrameResponse() {}
    explicit FrameResponse(int frameId, const Decision& decision = Decision(), const std::string& msg = std::string()) :
        frameId(frameId), decision(decision), msg(msg) {}

    bool isValid() const;
    std::string toString() const;

    bool connectionLost = false;
    bool received = false;

    int frameId = -1;
    Decision decision;
    std::string msg;

    // Mostly for HumanConnection.
    KeySet keySet;

    std::string mawashiArea;
    std::string original;
    int usec = 0;
};

#endif
