#ifndef WII_KEY_SENDER_H_
#define WII_KEY_SENDER_H_

#include "core/key_set.h"

class KeySender {
public:
    virtual ~KeySender() {}

    virtual void sendKey(const KeySet&) = 0;
    void sendKey(Key key) { sendKey(KeySet(key)); }

};

#endif
