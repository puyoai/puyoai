#include "core/client/connector/client_connector.h"

#include <iostream>
#include <sstream>
#include "core/client/connector/drop_decision.h"

using namespace std;

static string escapeMessage(string str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == ' ')
            str[i] = '_';
    }

    return str;
}

void ClientConnector::sendWithoutDecision(int frameId)
{
    cout << "ID=" << frameId << endl;
    LOG(INFO) << "ID=" << frameId << endl;
}

void ClientConnector::send(int frameId, const DropDecision& dropDecision)
{
    ostringstream ss;
    ss << "ID=" << frameId
       << " X=" << dropDecision.decision().x
       << " R=" << dropDecision.decision().r
       << " MSG=" << escapeMessage(dropDecision.message());

    string s = ss.str();
    cout << s << endl;
    LOG(INFO) << s;
}

FrameData ClientConnector::receive()
{
    std::string line, term;
    std::getline(std::cin, line);
    VLOG(1) << line;

    FrameData frameData;
    if (!std::cin) {
        frameData.connectionLost = true;
        return frameData;
    }

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

    return frameData;
}
