#include "core/client/ai/raw_ai.h"

#include <glog/logging.h>

#include "core/client/connector/stdio_client_connector.h"
#include "core/frame_request.h"
#include "core/frame_response.h"

RawAI::RawAI() :
    connector_(new StdioClientConnector)
{
}

void RawAI::runLoop()
{
    while (true) {
        FrameRequest frameRequest;
        if (!connector_->receive(&frameRequest)) {
            if (connector_->isClosed()) {
                LOG(INFO) << "connection is closed";
                break;
            }
            LOG(ERROR) << "received unexpected request?";
            break;
        }

        if (!frameRequest.isValid())
            continue;

        connector_->send(playOneFrame(frameRequest));
    }
}
