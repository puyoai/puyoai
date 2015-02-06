#include "core/server/connector/human_connector.h"

#include "core/frame_request.h"
#include "core/frame_response.h"
#include "core/key.h"

using namespace std;

void HumanConnector::write(const FrameRequest& req)
{
    writeString(req.toString());
}

void HumanConnector::writeString(const string& message)
{
    LOG(INFO) << message;
}

FrameResponse HumanConnector::read()
{
    FrameResponse fr;
    fr.received = true;

    lock_guard<mutex> lock(mu_);
    fr.keySet = currentKeySet_;
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
    currentKeySet_ = keySet;
}
