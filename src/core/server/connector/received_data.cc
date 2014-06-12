#include "core/server/connector/received_data.h"

#include <sstream>
#include <string>

using namespace std;

void ReceivedData::SerializeToString(string* output) const
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

bool ReceivedData::isValid() const
{
    if (!received)
        return false;

    return decision.isValid();
}
