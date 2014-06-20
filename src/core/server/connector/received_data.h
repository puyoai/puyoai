#ifndef CORE_SERVER_CONNECTOR_RECEIVED_DATA_H_
#define CORE_SERVER_CONNECTOR_RECEIVED_DATA_H_

#include <string>
#include <vector>

#include "core/decision.h"
#include "core/key.h"


// TODO(mayah): Should we split this to PipeConnectorReceivedData and
// HumanConnectorReceivedData? Or, split using Decision or Key?
class ReceivedData {
public:
    bool isValid() const;

    // TODO(mayah): Rename toString() and returns std::string instead.
    void SerializeToString(std::string* output) const;

    bool received = false;
    int frameId = -1;
    Decision decision;
    std::string msg;

    // For HumanConnector ReceivedData.
    // Basically the RecievedData of HumanConnector does not have any Decision.
    // Instead, we have key.
    Key key = Key::KEY_NONE;

    std::string mawashi_area;
    std::string original;
    int usec = 0;
};

#endif
