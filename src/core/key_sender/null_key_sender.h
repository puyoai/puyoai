#ifndef CORE_KEY_SENDER__NULL_KEY_SENDER_H_
#define CORE_KEY_SENDER__NULL_KEY_SENDER_H_

#include "base/base.h"
#include "core/key_set.h"
#include "core/key_sender/key_sender.h"

class NullKeySender : public KeySender {
public:
    virtual ~NullKeySender() {}

    virtual void sendWait(int /*ms*/) override {}
    virtual void sendKeySet(const KeySet&, bool /*forceSend*/) override {}
    virtual void sendKeySetSeq(const KeySetSeq&) override {}
};

#endif // CORE_KEY_SENDER__NULL_KEY_SENDER_H_
