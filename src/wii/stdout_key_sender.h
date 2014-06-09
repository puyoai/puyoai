#ifndef WII_STDOUT_KEY_SENDER_H_
#define WII_STDOUT_KEY_SENDER_H_

#include "base/base.h"
#include "core/key.h"
#include "wii/key_sender.h"

class StdoutKeySender : public KeySender {
public:
    virtual ~StdoutKeySender() {}

    virtual void sendKey(Key) OVERRIDE;
    virtual void sendKey(Key, Key) OVERRIDE;
};

#endif
