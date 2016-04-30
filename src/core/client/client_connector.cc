#include "core/client/client_connector.h"

#include <glog/logging.h>

#include <string>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

namespace {
const int kBufferSize = 1024;
} // namespace anonymous

ClientConnector::~ClientConnector()
{
}

bool ClientConnector::receive(FrameRequest* frameRequest)
{
    if (closed_)
        return false;

    FrameRequestHeader header;
    if (!impl_->readExactly(reinterpret_cast<char*>(&header), sizeof(header))) {
        LOG(ERROR) << "unexpected eof when reading header";
        return false;
    }

    if (header.size > kBufferSize) {
        LOG(ERROR) << "size too large: size=" << header.size;
        return false;
    }

    // TODO(mayah): This might cause buffer overflow.
    char payload[kBufferSize + 1];
    if (!impl_->readExactly(payload, header.size)) {
        LOG(ERROR) << "unepxected eof when reading payload";
        return false;
    }

    payload[header.size] = '\0';

    // TODO: Use LOG(INFO) for informative frames.
    VLOG(1) << "RECEIVED: " << payload;
    *frameRequest = FrameRequest::parsePayload(payload, header.size);
    return true;
}

void ClientConnector::send(const FrameResponse& resp)
{
    string s = resp.toString();

    // Send size as header.
    uint32_t size = s.size();
    if (!impl_->writeExactly(&size, sizeof(size))) {
        LOG(ERROR) << "failed to write header";
        return;
    }
    if (!impl_->writeExactly(s.data(), s.size())) {
        LOG(ERROR) << "failed to write body";
        return;
    }
    impl_->flush();

    if (resp.isValid()) {
        LOG(INFO) << "SEND: " << s;
    } else {
        VLOG(1) << "SEND: " << s;
    }
}
