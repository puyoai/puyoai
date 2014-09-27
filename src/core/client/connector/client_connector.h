#ifndef CLIENT_CONNECTION_CLIENT_CONNECTOR_H_
#define CLIENT_CONNECTION_CLIENT_CONNECTOR_H_

#include "core/frame_request.h"

class ClientFrameResponse;

class ClientConnector {
public:
    FrameRequest receive();
    void send(const ClientFrameResponse&);
};

#endif
