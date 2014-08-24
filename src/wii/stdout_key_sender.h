#ifndef WII_STDOUT_KEY_SENDER_H_
#define WII_STDOUT_KEY_SENDER_H_

#include "base/base.h"
#include "core/key_set.h"
#include "wii/key_sender.h"

class StdoutKeySender : public KeySender {
public:
    virtual ~StdoutKeySender() {}

    virtual void sendKey(const KeySet&) override;
};

#endif
