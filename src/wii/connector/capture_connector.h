#ifndef CAPTURE_CAPTURE_CONNECTOR_H_
#define CAPTURE_CAPTURE_CONNECTOR_H_

class CaptureConnector {
public:
    virtual ~CaptureConnector() {}

    virtual void sendKey(Key) = 0;
    virtual void sendKey(Key, Key) = 0;
};

#endif
