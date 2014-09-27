#include "core/client/connector/client_connector.h"

#include <iostream>
#include <sstream>

#include "core/client/connector/client_frame_response.h"

using namespace std;

static string escapeMessage(string str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == ' ')
            str[i] = '_';
    }

    return str;
}

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

void ClientConnector::send(const ClientFrameResponse& resp)
{
    ostringstream ss;
    ss << "ID=" << resp.frameId();
    if (resp.decision().isValid()) {
        ss << " X=" << resp.decision().x
           << " R=" << resp.decision().r;
    }
    if (!resp.message().empty()) {
        ss << " MSG=" << escapeMessage(resp.message());
    }

    string s = ss.str();
    cout << s << endl;
    LOG(INFO) << s;
}
