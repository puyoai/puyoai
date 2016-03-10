#ifndef CORE_SERVER_CONNECTOR_HUMAN_CONNECTOR_H_
#define CORE_SERVER_CONNECTOR_HUMAN_CONNECTOR_H_

#include <mutex>
#include <string>

#include "core/server/connector/server_connector.h"
#include "core/key_set.h"

struct FrameRequest;
struct FrameResponse;

class HumanConnector : public ServerConnector {
public:
    explicit HumanConnector(int player) : ServerConnector(player) {}
    virtual ~HumanConnector() override {}

    virtual void send(const FrameRequest&) override;
    virtual bool receive(FrameResponse*) override;

    virtual bool isHuman() const final { return true; }
    virtual bool isClosed() const final { return false; }

    void setKeySet(const KeySet&);

private:
    std::mutex mu_;
    KeySet currentKeySet_;
};

#endif // CORE_SERVER_CONNECTOR_HUMAN_CONNECTOR_H_
