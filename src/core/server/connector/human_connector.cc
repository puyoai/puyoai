#include "core/server/connector/human_connector.h"

#include "core/frame_request.h"
#include "core/frame_response.h"
#include "core/key.h"

using namespace std;

void HumanConnector::write(const FrameRequest& req)
{
    writeString(req.toString());

    lock_guard<mutex> lock(mu_);
    nextIsPlayable_ = req.myPlayerFrameRequest().state.playable;
}

void HumanConnector::writeString(const string& message)
{
    LOG(INFO) << message;
}

FrameResponse HumanConnector::read()
{
    FrameResponse fr;
    fr.received = true;

    // leftTurnKey or rightTurnKey should be prioritized.
    lock_guard<mutex> lock(mu_);
    fr.keySet = currentKeySet_;

    if (!nextIsPlayable_) {
        currentKeySet_.setKey(Key::LEFT_TURN, false);
        currentKeySet_.setKey(Key::RIGHT_TURN, false);
    }
    return fr;
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

    currentKeySet_.setKey(Key::DOWN, keySet.hasKey(Key::DOWN));
    currentKeySet_.setKey(Key::LEFT, keySet.hasKey(Key::LEFT));
    currentKeySet_.setKey(Key::RIGHT, keySet.hasKey(Key::RIGHT));

    // These key are bit-or. We will consume this only when playable state.
    // Otherwise, we often miss the key or uses too much.
    if (keySet.hasKey(Key::LEFT_TURN))
        currentKeySet_.setKey(Key::LEFT_TURN);
    if (keySet.hasKey(Key::RIGHT_TURN))
        currentKeySet_.setKey(Key::RIGHT_TURN);
}
