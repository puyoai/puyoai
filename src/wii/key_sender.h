#ifndef WII_KEY_SENDER_H_
#define WII_KEY_SENDER_H_

#include "core/key_set.h"

class KeySender {
public:
    virtual ~KeySender() {}

    void send(const KeySet& keySet, bool forceSend = false)
    {
        if (forceSend || keySet != keySetLastSent_) {
            keySetLastSent_ = keySet;
            sendKeySet(keySet);
        }
    }
    void send(Key key, bool forceSend = false) { send(KeySet(key), forceSend); }

protected:
    // TODO(mayah): Key::UP/Key::DOWN is chosen with probability.
    KeySender() : keySetLastSent_(KeySet(Key::UP, Key::DOWN)) {}
    virtual void sendKeySet(const KeySet&) = 0;

private:
    KeySet keySetLastSent_;
};

#endif
