#ifndef CLIENT_CONNECTION_CLIENT_CONNECTOR_H_
#define CLIENT_CONNECTION_CLIENT_CONNECTOR_H_

struct FrameRequest;
struct FrameResponse;

class ClientConnector {
public:
    FrameRequest receive();
    void send(const FrameResponse&);
};

#endif
