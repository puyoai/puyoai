#include "core/client/ai/raw_ai.h"

#include <glog/logging.h>

#include "core/frame_request.h"
#include "core/frame_response.h"

void RawAI::runLoop()
{
    while (true) {
        FrameRequest request = connector_.receive();
        if (request.connectionLost) {
            LOG(INFO) << "connection lost";
            break;
        }

        if (!request.isValid())
            continue;

        connector_.send(playOneFrame(request));
    }
}


