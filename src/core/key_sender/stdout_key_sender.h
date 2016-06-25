#ifndef CORE_KEY_SENDER_STDOUT_KEY_SENDER_H_
#define CORE_KEY_SENDER_STDOUT_KEY_SENDER_H_

#include "base/base.h"
#include "core/key_sender/key_sender.h"
#include "core/key_set.h"

class StdoutKeySender : public KeySender {
public:
    virtual ~StdoutKeySender() {}

    virtual void sendWait(int ms) override;
    virtual void sendKeySet(const KeySet&, bool forceSend) override;
    virtual void sendKeySetSeq(const KeySetSeq&) override;
};

#endif // CORE_KEY_SENDER_STDOUT_KEY_SENDER_H_
