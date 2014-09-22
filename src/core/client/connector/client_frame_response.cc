#include "core/client/connector/client_frame_response.h"

ClientFrameResponse::ClientFrameResponse(int frameId) :
    frameId_(frameId)
{
}

ClientFrameResponse::ClientFrameResponse(int frameId, const Decision& decision, const std::string& message) :
    frameId_(frameId),
    decision_(decision),
    message_(message)
{
}

