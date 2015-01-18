#ifndef CAPTURE_DRIVER_SYNTEK_H_
#define CAPTURE_DRIVER_SYNTEK_H_

#include <functional>
#include <libusb-1.0/libusb.h>

// TODO(mayah): noncopyable
// This class is not thread-safe.
class SyntekDriver {
public:
    ~SyntekDriver() {}

    // Initialize SyntekDriver and open it.
    // If failed, nullptr will be returned.
    // libusb must be initialized before this function.
    static SyntekDriver* open();
    bool close();

    typedef std::function<void (const unsigned char* higher,
                                const unsigned char* lower,
                                int bytesPerRow,
                                int numRowsPerBuffer)> ImageReceivedCallback;
    void setImageReceivedCallback(ImageReceivedCallback callback) { imageReceivedCallback_ = callback; }

    void runRead();

private: // make this private.
    SyntekDriver() {}

    static void release(int = 0);
    static void dataReceivedCallback(libusb_transfer*);

    bool initAll();

    bool claimInterface();
    bool initDevice();
    bool initChip();
    bool initAudio();
    void logDriverVersion();
    bool initResolution();
    bool setAlternateInterface(uint8_t alternateSetting);
    bool setStreaming(bool flag);

    void dataReceived(libusb_transfer*);

    bool readIndex(int index, uint8_t* value);
    bool writeIndex(int index, int value);

    bool readSAA711XRegister(uint8_t reg, uint8_t* outVal);
    bool writeSAA711XRegister(uint8_t reg, int16_t val);
    bool SAA711XExpect(uint8_t val);

    bool initializeVT1612AChip();
    bool writeVT1612ARegister(uint8_t reg, uint16_t val);

    static const int BUFFER_SIZE = 1440 * 240;

    static std::unique_ptr<SyntekDriver> s_driver;
    libusb_device_handle* deviceHandle_ = nullptr;

    int pendingRequests_ = 0;
    int highOffset_ = 0;
    int lowOffset_ = 0;
    unsigned char highBuffer_[BUFFER_SIZE];
    unsigned char lowBuffer_[BUFFER_SIZE];
    ImageReceivedCallback imageReceivedCallback_;
};

#endif
