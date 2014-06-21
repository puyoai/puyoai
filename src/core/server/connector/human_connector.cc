#include "core/server/connector/human_connector.h"

#include "core/key.h"

using namespace std;

void HumanConnector::write(const std::string& message)
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
    currentKeySet_ = keySet;
}
