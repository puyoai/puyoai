#include "core/frame_response.h"

#include <cstddef>
#include <sstream>
#include <string>

using namespace std;

static string unescapeMessage(const string& str)
{
    string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '_') {
            result.push_back(' ');
        } else if (str[i] == ',') {
            result.push_back('\n');
        } else {
            result.push_back(str[i]);
        }
    }

    return result;
}

static string escapeMessage(const string& str)
{
    string result;
    for (char c : str) {
        if (c == ' ')
            result.push_back('_');
        else if (c == '\n')
            result.push_back(',');
        else
            result.push_back(c);
    }

    return result;
}

// static
FrameResponse FrameResponse::parse(const string& str)
{
    std::istringstream iss(str);
    std::string tmp;

    FrameResponse data;

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
        } else if (tmp.substr(0, 3) == "PX=") {
            std::istringstream istr(tmp.c_str() + 3);
            istr >> data.preDecision.x;
        } else if (tmp.substr(0, 3) == "PR=") {
            std::istringstream istr(tmp.c_str() + 3);
            istr >> data.preDecision.r;
        } else if (tmp.substr(0, 4) == "MSG=") {
            data.message = unescapeMessage(tmp.c_str() + 4);
        } else if (tmp.substr(0, 3) == "MA=") {
            data.mawashiArea = tmp.c_str() + 3;
        }
    }

    return data;
}

bool FrameResponse::isValid() const
{
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
    if (preDecision.isValid()) {
        ss << " PX=" << preDecision.x
           << " PR=" << preDecision.r;
    }
    if (!message.empty()) {
        ss << " MSG=" << escapeMessage(message);
    }

    return ss.str();
}
