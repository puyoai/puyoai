#ifndef CLIENT_CONNECTION_CLIENT_CONNECTOR_H_
#define CLIENT_CONNECTION_CLIENT_CONNECTOR_H_

struct FrameRequest;
struct FrameResponse;

class ClientConnector {
public:
    // Returns true if receive suceeded.
    bool receive(FrameRequest* request);
    void send(const FrameResponse&);

    bool isClosed() { return closed_; }

private:
    bool closed_ = false;
};

#endif
