#ifndef CORE_SERVER_CONNECTOR_RECEIVED_DATA_H_
#define CORE_SERVER_CONNECTOR_RECEIVED_DATA_H_

#include <string>
#include <vector>

#include "core/decision.h"
#include "core/key.h"
#include "core/kumipuyo.h"

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

// TODO(mayah): Why does this class have player_id and is_human?
class PlayerLog {
public:
    int frame_id;
    int player_id;
    std::vector<ReceivedData> received_data;
    bool is_human;

    void SerializeToString(std::string* output) const;
};

#endif
