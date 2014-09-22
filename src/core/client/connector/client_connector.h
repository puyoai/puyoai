#ifndef CLIENT_CONNECTION_CLIENT_CONNECTOR_H_
#define CLIENT_CONNECTION_CLIENT_CONNECTOR_H_

#include "core/frame_data.h"

class ClientFrameResponse;

class ClientConnector {
public:
    FrameData receive();
    void send(const ClientFrameResponse&);
};

#endif
