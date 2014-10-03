#include "core/client/connector/client_connector.h"

#include <iostream>
#include <sstream>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

FrameRequest ClientConnector::receive()
{
    FrameRequest frameRequest;
    std::string line;
    while (true) {
        if (!std::getline(std::cin, line)) {
            frameRequest.connectionLost = true;
            return frameRequest;
        }

        if (line != "")
            break;
    }
    VLOG(1) << line;

    return FrameRequest::parse(line);
}

void ClientConnector::send(const FrameResponse& resp)
{
    string s = resp.toString();
    cout << s << endl;
    VLOG(1) << s;
}
