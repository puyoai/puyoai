#include "capture/connector/capture_connector.h"

#include "base/base.h"

class NullCaptureConnector : public CaptureConnector {
public:
    virtual ~NullCaptureConnector() {}

    virtual void sendKey(Key) OVERRIDE;
    virtual void sendKey(Key, Key) OVERRIDE;
};
