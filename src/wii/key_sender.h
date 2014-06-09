#ifndef WII_KEY_SENDER_H_
#define WII_KEY_SENDER_H_

#include "core/key.h"

class KeySender {
public:
    virtual ~KeySender() {}

    virtual void sendKey(Key) = 0;
    virtual void sendKey(Key, Key) = 0;
};

#endif
