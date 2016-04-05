#include "core/client/connector/client_connector.h"

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
    if (!readExactly(reinterpret_cast<char*>(&header), sizeof(header))) {
        LOG(ERROR) << "unexpected eof when reading header";
        return false;
    }

    if (header.size > kBufferSize) {
        LOG(ERROR) << "size too large: size=" << header.size;
        return false;
    }

    char payload[kBufferSize + 1];
    if (!readExactly(payload, header.size)) {
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
    writeExactly(&size, sizeof(size));
    writeExactly(s.data(), s.size());
    flush();

    if (resp.isValid())
      LOG(INFO) << "SEND: " << s;
    else
      VLOG(1) << "SEND: " << s;
}
