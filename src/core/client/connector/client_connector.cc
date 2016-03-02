#include "core/client/connector/client_connector.h"

#include <glog/logging.h>

#include <iostream>
#include <string>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

namespace {
const int kBufferSize = 1024;
} // namespace anonymous

bool ClientConnector::receive(FrameRequest* frameRequest)
{
    if (closed_)
        return false;

    FrameRequestHeader header;
    if (!cin.read(reinterpret_cast<char*>(&header), sizeof(header))) {
        LOG(ERROR) << "unexpected eof when reading header";
        return false;
    }

    if (header.size > kBufferSize) {
        LOG(ERROR) << "size too large: size=" << header.size;
        return false;
    }

    char payload[kBufferSize + 1];
    if (!cin.read(payload, header.size)) {
        LOG(ERROR) << "unepxected eof when reading payload";
        return false;
    }

    payload[header.size] = '\0';

    LOG(INFO) << "RECEIVED: " << payload;
    *frameRequest = FrameRequest::parsePayload(payload, header.size);
    return true;
}

void ClientConnector::send(const FrameResponse& resp)
{
    string s = resp.toString();

    // Send size as header.
    uint32_t size = s.size();
    cout.write(reinterpret_cast<char*>(&size), sizeof(size));
    cout.write(s.data(), s.size());
    cout.flush();

    LOG(INFO) << "SEND: " << s;
}
