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

FrameData ClientConnector::receive()
{
    FrameData frameData;
    std::string line;
    while (true) {
        if (!std::getline(std::cin, line)) {
            frameData.connectionLost = true;
            return frameData;
        }

        if (line != "")
            break;
    }
    VLOG(1) << line;

    string term;
    for (std::istringstream iss(line); iss >> term;) {
        if (term.find('=') == std::string::npos)
            continue;

        const char* key = term.c_str();
        const char* value = term.c_str() + term.find('=') + 1;
        if (strncmp(key, "STATE", 5) == 0) {
            int state = strtoull(value, NULL, 10);
            frameData.playerFrameData[0].userState.parseFromDeprecatedState(state);
            frameData.playerFrameData[1].userState.parseFromDeprecatedState(state >> 1);
            continue;
        } else if (strncmp(key, "ID", 2) == 0) {
            frameData.valid = true;
            frameData.id = std::atoi(value);
            continue;
        } else if (strncmp(key, "END", 3) == 0) {
            frameData.gameEnd = std::atoi(value);
        }

        PlayerFrameData& playerFrameData = frameData.playerFrameData[(key[0] == 'Y') ? 0 : 1];
        switch (key[1]) {
        case 'F':
            playerFrameData.field = PlainField(string(value));
            break;
        case 'P':
            playerFrameData.kumipuyoSeq = KumipuyoSeq(string(value));
            break;
        case 'S':
            playerFrameData.score = std::atoi(value);
            break;
        case 'X':
            playerFrameData.kumipuyoPos.x = std::atoi(value);
            break;
        case 'Y':
            playerFrameData.kumipuyoPos.y = std::atoi(value);
            break;
        case 'R':
            playerFrameData.kumipuyoPos.r = std::atoi(value);
            break;
        case 'O':
            playerFrameData.ojama = std::atoi(value);
            break;
        }
    }

    VLOG(1) << frameData.toString();

    return frameData;
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
