#include "core/client/connector/client_connector.h"

#include <glog/logging.h>

#include <iostream>
#include <string>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

bool ClientConnector::receive(FrameRequest* frameRequest)
{
    if (closed_)
        return false;

    std::string line;
    while (true) {
        if (!std::getline(std::cin, line)) {
            closed_ = true;
            return false;
        }

        if (line != "")
            break;
    }

    LOG(INFO) << line;
    *frameRequest = FrameRequest::parse(line);
    return true;
}

void ClientConnector::send(const FrameResponse& resp)
{
    string s = resp.toString();
    cout << s << endl;
    LOG(INFO) << s;
}
