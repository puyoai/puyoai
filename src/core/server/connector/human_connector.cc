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
    cfr.keySet = currentKeySet_;

    if (!nextIsPlayable_) {
        currentKeySet_.setKey(Key::KEY_LEFT_TURN, false);
        currentKeySet_.setKey(Key::KEY_RIGHT_TURN, false);
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

    currentKeySet_.setKey(Key::KEY_DOWN, keySet.hasKey(Key::KEY_DOWN));
    currentKeySet_.setKey(Key::KEY_LEFT, keySet.hasKey(Key::KEY_LEFT));
    currentKeySet_.setKey(Key::KEY_RIGHT, keySet.hasKey(Key::KEY_RIGHT));

    // These key are bit-or. We will consume this only when playable state.
    // Otherwise, we often miss the key or uses too much.
    if (keySet.hasKey(Key::KEY_LEFT_TURN))
        currentKeySet_.setKey(Key::KEY_LEFT_TURN);
    if (keySet.hasKey(Key::KEY_RIGHT_TURN))
        currentKeySet_.setKey(Key::KEY_RIGHT_TURN);
}
