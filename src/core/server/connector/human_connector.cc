#include "core/server/connector/human_connector.h"

#include "core/key.h"

using namespace std;

void HumanConnector::write(const std::string& message)
{
    LOG(INFO) << message;
}

ReceivedData HumanConnector::read()
{
    ReceivedData rd;
    rd.received = true;

    // leftTurnKey or rightTurnKey should be prioritized.
    lock_guard<mutex> lock(mu_);
    if (currentKeySet_.downKey)
        rd.key = Key::KEY_DOWN;
    if (currentKeySet_.leftKey)
        rd.key = Key::KEY_LEFT;
    if (currentKeySet_.rightKey)
        rd.key = Key::KEY_RIGHT;
    if (currentKeySet_.leftTurnKey)
        rd.key = Key::KEY_LEFT_TURN;
    if (currentKeySet_.rightTurnKey)
        rd.key = Key::KEY_RIGHT_TURN;
    return rd;
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
