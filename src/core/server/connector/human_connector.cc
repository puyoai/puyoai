#include "core/server/connector/human_connector.h"

#include "core/key.h"
#include "core/server/connector/connector_frame_request.h"

using namespace std;

void HumanConnector::write(const ConnectorFrameRequest& req)
{
    writeString(req.toRequestString(playerId_));

    lock_guard<mutex> lock(mu_);
    nextIsPlayable_ = req.userState[playerId_].playable;
}

void HumanConnector::writeString(const string& message)
{
    LOG(INFO) << message;
}

ConnectorFrameResponse HumanConnector::read()
{
    ConnectorFrameResponse cfr;
    cfr.received = true;

    // leftTurnKey or rightTurnKey should be prioritized.
    lock_guard<mutex> lock(mu_);
    if (currentKeySet_.downKey)
        cfr.key = Key::KEY_DOWN;
    if (currentKeySet_.leftKey)
        cfr.key = Key::KEY_LEFT;
    if (currentKeySet_.rightKey)
        cfr.key = Key::KEY_RIGHT;
    if (currentKeySet_.leftTurnKey)
        cfr.key = Key::KEY_LEFT_TURN;
    if (currentKeySet_.rightTurnKey)
        cfr.key = Key::KEY_RIGHT_TURN;

    if (!nextIsPlayable_) {
        currentKeySet_.leftTurnKey = false;
        currentKeySet_.rightTurnKey = false;
    }
    return cfr;
}

void HumanConnector::setAlive(bool)
{
    CHECK(false) << "HumanConnector does not have alive flag.";
}

int HumanConnector::readerFd() const
{
    CHECK(false) << "HumanConnector does not have reader file descriptor.";
    return -1;
}

void HumanConnector::setKeySet(const KeySet& keySet)
{
    lock_guard<mutex> lock(mu_);

    currentKeySet_.downKey = keySet.downKey;
    currentKeySet_.leftKey = keySet.leftKey;
    currentKeySet_.rightKey = keySet.rightKey;

    // These key are bit-or.
    currentKeySet_.leftTurnKey |= keySet.leftTurnKey;
    currentKeySet_.rightTurnKey |= keySet.rightTurnKey;
}
