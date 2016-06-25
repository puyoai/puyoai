#ifndef WII_SERIAL_KEY_SENDER_H_
#define WII_SERIAL_KEY_SENDER_H_

#include <termios.h>
#include <string>

#include "base/base.h"
#include "wii/key_sender.h"

class SerialKeySender : public KeySender {
public:
    explicit SerialKeySender(const std::string& deviceName);
    virtual ~SerialKeySender();

    virtual void sendWait(int ms) override;
    virtual void sendKeySet(const KeySet&, bool forceSend) override;
    virtual void sendKeySetSeq(const KeySetSeq&) override;

private:
    void sendKeySetInternal(const KeySet&);

    int fd_;
    struct termios original_;
    KeySet lastSent_;
};

#endif
