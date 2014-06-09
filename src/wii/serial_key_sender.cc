#include "wii/serial_key_sender.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glog/logging.h>
#include <iostream>

using namespace std;

SerialKeySender::SerialKeySender(const string& deviceName)
{
    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = CS8 | CLOCAL | CREAD;
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;

    if ((fd_ = open(deviceName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {
        PLOG(FATAL) << "Failed to open " << deviceName;
    }

    cout << fd_ << endl;
    CHECK_EQ(tcgetattr(fd_, &original_), 0) << "Error to get tty attribute";

    cfsetospeed(&tio, B38400);
    cfsetispeed(&tio, B38400);

    if (tcflush(fd_, TCIFLUSH) < 0) {
        PLOG(FATAL) << "tcflush";
    }

    if (tcsetattr(fd_, TCSANOW, &tio) < 0) {
        PLOG(FATAL) << "tcsetattr";
    }

    if (fcntl(fd_, F_SETFL, FNDELAY) < 0) {
        PLOG(FATAL) << "fcntl";
    }
}

SerialKeySender::~SerialKeySender()
{
    if (tcdrain(fd_) < 0) {
        PLOG(FATAL) << "tcdrain";
    }

    if (tcsetattr(fd_, TCSANOW, &original_) < 0) {
        PLOG(FATAL) << "tcsetattr";
    }

    if (close(fd_) < 0) {
        PLOG(FATAL) << "close";
    }
}

void SerialKeySender::sendKey(Key k)
{
    sendKey(k, KEY_NONE);
}

void SerialKeySender::sendKey(Key k1, Key k2)
{
    unsigned char c = 0;

    if (k1 != KEY_NONE) {
        int kk = static_cast<int>(k1);
        CHECK(1 <= kk && kk <= 8) << k1;
        c |= 1 << (kk - 1);
    }
    if (k2 != KEY_NONE) {
        int kk = static_cast<int>(k2);
        CHECK(1 <= kk && kk <= 8) << k2;
        c |= 1 << (kk - 1);
    }

    CHECK_EQ(write(fd_, &c, sizeof(unsigned char)), 1);

    cout << toString(k1) << " " << toString(k2) << " " << int(c) << endl;
}
