#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>

using namespace std;

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void run(int fd)
{
    while (1) {
        // TODO: Don't do busy loop.
        if (!kbhit())
            continue;

        unsigned char key = getchar();

        if (key == 27) {
            key = getchar();
            key = getchar();
            uint8_t x;
            switch (key) {
            case 65:
                cout << "up" << endl;
                x = 1 << 0;
                write(fd, &x, sizeof(uint8_t));
                x = 0;
                write(fd, &x, sizeof(uint8_t));
                break;
            case 66:
                cout << "down" << endl;
                x = 1 << 2;
                write(fd, &x, sizeof(uint8_t));
                x = 0;
                write(fd, &x, sizeof(uint8_t));
                break;
            case 67:
                cout << "right" << endl;
                x = 1 << 1;
                write(fd, &x, sizeof(uint8_t));
                x = 0;
                write(fd, &x, sizeof(uint8_t));
                break;
            case 68:
                cout << "left" << endl;
                x = 1 << 3;
                write(fd, &x, sizeof(uint8_t));
                x = 0;
                write(fd, &x, sizeof(uint8_t));
                break;
            default:
                cout << "? " << (int)key << endl;
            }
        } else if (key == 'a') {
            cout << "a detected" << endl;
            uint8_t x = 1 << 4;
            write(fd, &x, sizeof(uint8_t));
            x = 0;
            write(fd, &x, sizeof(uint8_t));
        } else if (key == 'b') {
            cout << "b detected" << endl;
            uint8_t x = 1 << 5;
            write(fd, &x, sizeof(uint8_t));
            x = 0;
            write(fd, &x, sizeof(uint8_t));
        } else if (key == 10) {
            cout << "enter detected" << endl;
            uint8_t x = 1 << 6;
            write(fd, &x, sizeof(uint8_t));
            x = 0;
            write(fd, &x, sizeof(uint8_t));
        } else {
            cout << "?? " << (int)key << endl;
        }
    }
}

int main(void)
{
    const string deviceName = "/dev/tty.usbmodem1411";
    int fd_ = -1;

    struct termios tio;
    struct termios original_;
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = CS8 | CLOCAL | CREAD;
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;

    if ((fd_ = open(deviceName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {
        perror("open");
        return 1;
    }

    cout << fd_ << endl;
    if (tcgetattr(fd_, &original_) < 0) {
        perror("Error to get tty attribute");
        return 1;
    }

    cfsetospeed(&tio, B38400);
    cfsetispeed(&tio, B38400);

    if (tcflush(fd_, TCIFLUSH) < 0) {
        perror("tcflush");
        return 1;
    }

    if (tcsetattr(fd_, TCSANOW, &tio) < 0) {
        perror("tcsetattr");
        return 1;
    }

    if (fcntl(fd_, F_SETFL, FNDELAY) < 0) {
        perror("fcntl");
        return 1;
    }

    run(fd_);

    if (tcdrain(fd_) < 0) {
        perror("tcdrain");
        return 1;
    }

    if (tcsetattr(fd_, TCSANOW, &original_) < 0) {
        perror("tcsetattr");
        return 1;
    }

    if (close(fd_) < 0) {
        perror("close");
        return 1;
    }
}
