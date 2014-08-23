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

    virtual void sendKey(Key) override;
    virtual void sendKey(Key, Key) override;

private:
    int fd_;
    struct termios original_;
};

#endif
