#ifndef WII_KEY_SENDER_H_
#define WII_KEY_SENDER_H_

#include "core/key_set_seq.h"

class KeySender {
public:
    virtual ~KeySender() {}

    virtual void sendWait(int ms) = 0;
    virtual void sendKeySet(const KeySet&, bool forceSend = false) = 0;
    virtual void sendKeySetSeq(const KeySetSeq& keySetSeq) = 0;
};

#endif
