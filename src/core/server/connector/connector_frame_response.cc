#include "core/server/connector/connector_frame_response.h"

#include <sstream>
#include <string>

using namespace std;

void ConnectorFrameResponse::SerializeToString(string* output) const
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
    output->append(ss.str());
}

bool ConnectorFrameResponse::isValid() const
{
    if (!received)
        return false;

    return decision.isValid();
}
