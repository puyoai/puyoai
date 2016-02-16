#include "core/server/connector/human_connector.h"

#include <glog/logging.h>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

void HumanConnector::send(const FrameRequest& req)
{
    LOG(INFO) << req.toString();
}

bool HumanConnector::receive(FrameResponse* response)
{
    *response = FrameResponse();

    lock_guard<mutex> lock(mu_);
    response->keySet = currentKeySet_;
    return true;
}

void HumanConnector::setKeySet(const KeySet& keySet)
{
    lock_guard<mutex> lock(mu_);
    currentKeySet_ = keySet;
}
