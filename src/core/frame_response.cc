#include "core/frame_response.h"

#include <sstream>

using namespace std;

static string unescapeMessage(string str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '_')
            str[i] = ' ';
    }

    return str;
}

static string escapeMessage(string str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == ' ')
            str[i] = '_';
    }

    return str;
}

// static
FrameResponse FrameResponse::parse(const char* str)
{
    std::istringstream iss(str);
    std::string tmp;

    FrameResponse data;
    data.received = true;
    data.original = std::string(str);

    while (getline(iss, tmp, ' ')) {
        if (tmp.substr(0, 3) == "ID=") {
            std::istringstream istr(tmp.c_str() + 3);
            istr >> data.frameId;
        } else if (tmp.substr(0, 2) == "X=") {
            std::istringstream istr(tmp.c_str() + 2);
            istr >> data.decision.x;
        } else if (tmp.substr(0, 2) == "R=") {
            std::istringstream istr(tmp.c_str() + 2);
            istr >> data.decision.r;
        } else if (tmp.substr(0, 4) == "MSG=") {
            data.msg = unescapeMessage(tmp.c_str() + 4);
        } else if (tmp.substr(0, 3) == "MA=") {
            data.mawashiArea = tmp.c_str() + 3;
        }
    }

    return data;
}

bool FrameResponse::isValid() const
{
    if (connectionLost || !received)
        return false;

    return decision.isValid();
}

std::string FrameResponse::toString() const
{
    ostringstream ss;
    ss << "ID=" << frameId;
    if (decision.isValid()) {
        ss << " X=" << decision.x
           << " R=" << decision.r;
    }
    if (!msg.empty()) {
        ss << " MSG=" << escapeMessage(msg);
    }

    return ss.str();
}
