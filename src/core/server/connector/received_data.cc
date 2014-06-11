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

void PlayerLog::SerializeToString(string* output) const {
    stringstream ss;
    ss << "\n";
    ss << "{";
    ss << "'frame_id':";
    ss << frame_id;
    ss << ",";
    ss << "'player_id':";
    ss << player_id;
    ss << ",";
    ss << "'received_data':[";
    for (size_t i = 0; i < received_data.size(); i++) {
        if (i > 0) {
            ss << ",";
        }
        string tmp;
        received_data[i].SerializeToString(&tmp);
        ss << tmp;
    }
    ss << "]";
    ss << "}";
    output->append(ss.str());
}
