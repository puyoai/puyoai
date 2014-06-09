#ifndef CLIENT_CONNECTION_CLIENT_CONNECTOR_H_
#define CLIENT_CONNECTION_CLIENT_CONNECTOR_H_

#include "core/frame_data.h"

class DropDecision;

class ClientConnector {
public:
    FrameData receive();

    void sendWithoutDecision(int frameId);
    void send(int frameId, const DropDecision&);
};

#endif
