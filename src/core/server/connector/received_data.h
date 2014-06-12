#ifndef CORE_SERVER_CONNECTOR_RECEIVED_DATA_H_
#define CORE_SERVER_CONNECTOR_RECEIVED_DATA_H_

#include <string>
#include <vector>

#include "core/decision.h"

class ReceivedData {
public:
    bool isValid() const;

    bool received = false;
    int frameId = -1;
    Decision decision;
    std::string msg;

    std::string mawashi_area;
    std::string original;
    int usec = 0;

    void SerializeToString(std::string* output) const;
};

#endif
