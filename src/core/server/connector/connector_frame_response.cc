#include "core/server/connector/connector_frame_response.h"

#include <sstream>
#include <string>

using namespace std;

// static
ConnectorFrameResponse ConnectorFrameResponse::parse(const char* str)
{
    std::istringstream iss(str);
    std::string tmp;

    ConnectorFrameResponse data;

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
            data.msg = tmp.c_str() + 4;
        } else if (tmp.substr(0, 3) == "MA=") {
            data.mawashi_area = tmp.c_str() + 3;
        }
    }

    return data;
}

std::string ConnectorFrameResponse::toString() const
{
    stringstream ss;
    ss << "{";
    if (!original.empty()) {
        ss << "'original':";
        ss << "'" << original << "'";
        ss << ",";
    }
    if (decision.isValid()) {
        ss << "'decision':";
        ss << "{'x':" << decision.x << ",'r':" << decision.r << "}";
        ss << ",";
    }
    ss << "'frame_id':";
    ss << frameId;
    ss << "}";

    return ss.str();
}

bool ConnectorFrameResponse::isValid() const
{
    if (!received)
        return false;

    return decision.isValid();
}
