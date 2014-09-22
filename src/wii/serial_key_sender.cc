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

void SerialKeySender::sendKeySet(const KeySet& keySet)
{
    unsigned char c = static_cast<unsigned int>(keySet.toInt());
    CHECK_EQ(write(fd_, &c, sizeof(unsigned char)), 1);
    cout << keySet.toString() << " " << static_cast<int>(c) << endl;
}
