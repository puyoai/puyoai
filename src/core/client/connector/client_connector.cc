#include "core/client/connector/client_connector.h"

#include <iostream>
#include "core/client/connector/drop_decision.h"

using namespace std;

void ClientConnector::sendWithoutDecision(int frameId)
{
    cout << "ID=" << frameId << endl;
}

void ClientConnector::send(int frameId, const DropDecision& dropDecision)
{
    stringstream ss;
    ss << "ID=" << frameId
       << " X=" << dropDecision.decision().x
       << " R=" << dropDecision.decision().r
       << " MSG=" << dropDecision.message();

    string s = ss.str();
    cout << s << endl;
    LOG(INFO) << s;
}

// -1 if failed
// 0 if invalid
// 1 if valid
FrameData ClientConnector::receive()
{
    std::string line, term;
    std::getline(std::cin, line);

    // LOG(INFO) << line;

    FrameData frameData;

    if (!std::cin) {
        frameData.connectionLost = true;
        return frameData;
    }

    VLOG(1) << line;

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

    return frameData;
}
