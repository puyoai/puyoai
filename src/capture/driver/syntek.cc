#include "capture/driver/syntek.h"

#include <signal.h>
#include <unistd.h>

#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <glog/logging.h>

#include "base/base.h"
#include "capture/usb_device.h"

const int VENDOR = 0x05e1;
const int PRODUCT = 0x0408;

enum {
    SAA711XAUTO0AutomaticChrominanceStandardDetection = 1 << 1,
    SAA711XCSTDPAL_BGDHI   = 0 << 4,
    SAA711XCSTDNTSC44350Hz = 1 << 4,
    SAA711XCSTDPALN        = 2 << 4,
    SAA711XCSTDNTSCN       = 3 << 4,
    SAA711XCSTDNTSCJ       = 4 << 4,
    SAA711XCSTDSECAM       = 5 << 4,

    SAA711XCSTDNTSCM       = SAA711XCSTDPAL_BGDHI,
    SAA711XCSTDPAL60Hz     = SAA711XCSTDNTSC44350Hz,
    SAA711XCSTDNTSC44360Hz = SAA711XCSTDPALN,
    SAA711XCSTDPALM        = SAA711XCSTDNTSCN,
};
enum {
    SAA711XMODECompositeAI11 = 0,
    SAA711XMODECompositeAI12 = 1,
    SAA711XMODECompositeAI21 = 2,
    SAA711XMODECompositeAI22 = 3,
    SAA711XMODECompositeAI23 = 4,
    SAA711XMODECompositeAI24 = 5,
    SAA711XMODESVideoAI11_GAI2 = 6,
    SAA711XMODESVideoAI12_GAI2 = 7,
    SAA711XMODESVideoAI11_YGain = 8,
    SAA711XMODESVideoAI12_YGain = 9,
};
enum {
    SAA711XFUSE0Antialias = 1 << 6,
    SAA711XFUSE1Amplifier = 1 << 7,
};
enum {
    SAA711XGAI18StaticGainControl1 = 1 << 0,
    SAA711XGAI28StaticGainControl2 = 1 << 1,
    SAA711XGAFIXGainControlUserProgrammable = 1 << 2,
    SAA711XHOLDGAutomaticGainControlEnabled = 0 << 3,
    SAA711XHOLDGAutomaticGainControlDisabled = 1 << 3,
    SAA711XCPOFFColorPeakControlDisabled = 1 << 4,
    SAA711XVBSLLongVerticalBlanking = 1 << 5,
    SAA711XHLNRSReferenceSelect = 1 << 6,
};
enum {
    SAA711XYCOMBAdaptiveLuminanceComb = 1 << 6,
    SAA711XBYPSChrominanceTrapCombBypass = 1 << 7,
};
enum {
    SAA711XVNOIVerticalNoiseReductionNormal = 0 << 0,
    SAA711XVNOIVerticalNoiseReductionFast = 1 << 0,
    SAA711XVNOIVerticalNoiseReductionFree = 2 << 0,
    SAA711XVNOIVerticalNoiseReductionBypass = 3 << 0,
    SAA711XHTCHorizontalTimeConstantTVMode = 0 << 3,
    SAA711XHTCHorizontalTimeConstantVTRMode = 1 << 3,
    SAA711XHTCHorizontalTimeConstantAutomatic = 2 << 3,
    SAA711XHTCHorizontalTimeConstantFastLocking = 3 << 3,
    SAA711XFOETForcedOddEventToggle = 1 << 5,
    SAA711XFSELManualFieldSelection50Hz = 0 << 6,
    SAA711XFSELManualFieldSelection60Hz = 1 << 6,
    SAA711XAUFDAutomaticFieldDetection = 1 << 7,
};
enum {
    SAA711XCGAINChromaGainValueMinimum = 0x00,
    SAA711XCGAINChromaGainValueNominal = 0x2a,
    SAA711XCGAINChromaGainValueMaximum = 0x7f,
    SAA711XACGCAutomaticChromaGainControlEnabled = 0 << 7,
    SAA711XACGCAutomaticChromaGainControlDisabled = 1 << 7,
};
enum {
    SAA711XRTP0OutputPolarityInverted = 1 << 3,
};
enum {
    SAA711XSLM1ScalerDisabled = 1 << 1,
    SAA711XSLM3AudioClockGenerationDisabled = 1 << 3,
    SAA711XCH1ENAD1X = 1 << 6,
    SAA711XCH2ENAD2X = 1 << 7,
};
enum {
    SAA711XCCOMBAdaptiveChrominanceComb = 1 << 0,
    SAA711XFCTCFastColorTimeConstant = 1 << 2,
};

enum {
    VT1612ARegisterVolumeStereoOut = 0x02,
    VT1612ARegisterVolumeLineIn = 0x10,
    VT1612ARegisterRecordSelect = 0x1a,
    VT1612ARegisterRecordGain = 0x1c,
    VT1612ARegisterVendorID1 = 0x7c,
    VT1612ARegisterVendorID2 = 0x7e,
};
enum {
    VT1612AMute = 1 << 15,
};
enum {
    VT1612ARecordSourceMic = 0,
    VT1612ARecordSourceCD = 1,
    VT1612ARecordSourceVideoIn = 2,
    VT1612ARecordSourceAuxIn = 3,
    VT1612ARecordSourceLineIn = 4,
    VT1612ARecordSourceStereoMix = 5,
    VT1612ARecordSourceMonoMix = 6,
    VT1612ARecordSourcePhone = 7,
};
enum {
    STK0408StatusRegistryIndex = 0x100,
};
enum {
    STK0408StatusStreaming = 1 << 7,
};
enum {
    ECVSTK1160HighFieldFlag = 1 << 6,
    ECVSTK1160NewImageFlag = 1 << 7,
};

using namespace std;

// TODO: move to somewhere.
template<typename T>
T clamp(T left, T x, T right) { return std::min(std::max(left, x), right); }

std::unique_ptr<SyntekDriver> SyntekDriver::s_driver;

// ----------------------------------------------------------------------

// static
SyntekDriver* SyntekDriver::open()
{
    // TODO(mayah): shouldn't I call libusb_init here?
    libusb_init(NULL);

    if (s_driver.get())
        return s_driver.get();

    s_driver.reset(new SyntekDriver);
    if (!s_driver->initAll()) {
        s_driver.reset();
        return nullptr;
    }

    LOG(INFO) << "syntek driver is initialized.";
    return s_driver.get();
}

bool SyntekDriver::close()
{
    // Don't exit when something failed, since we'd like to call libusb_close surely.

    bool ok = true;
    if (!setStreaming(false)) {
        LOG(ERROR) << "failed to set streaming(false)";
        ok = false;
    }

    if (!setAlternateInterface(0)) {
        LOG(ERROR) << "failed to setAlternateInterface(0)";
        ok = false;
    }

    if (libusb_release_interface(s_driver->deviceHandle_, 0) < 0) {
        LOG(ERROR) << "failed to release interface";
        ok = false;
    }

    libusb_close(s_driver->deviceHandle_);

    return ok;
}

bool SyntekDriver::initAll()
{
    if (!claimInterface()) {
        LOG(ERROR) << "failed to claimInterface()";
        return false;
    }

    if (!initDevice()) {
        LOG(ERROR) << "failed to initDevice()";
        return false;
    }

    if (!initChip()) {
        LOG(ERROR) << "failed to initChip()";
        return false;
    }

    logDriverVersion();

    if (!initAudio()) {
        LOG(ERROR) << "failed to initialize audio";
        return false;
    }

    if (true) {  // S-VIDEO
    } else {  // COMPOSITE
        // (void)dev_stk0408_write0(device, 1 << 7 | 0x3 << 3, 1 << 7 | [self hardwareSource] << 3);
    }

    if (!initResolution()) {
        LOG(ERROR) << "failed to initialize resolution";
        return false;
    }

    if (!setAlternateInterface(5)) {
        LOG(ERROR) << "failed to setAlternateInterface(5)";
        return false;
    }

    if (!setStreaming(true)) {
        LOG(ERROR) << "failed to set streaming(true)";
        return false;
    }

    return true;
}

// static
void SyntekDriver::release(int)
{
    if (!s_driver.get())
        return;

    s_driver->close();
    s_driver.reset(nullptr);
}

bool SyntekDriver::claimInterface()
{
    libusb_device* dev = findDevice(VENDOR, PRODUCT);
    if (!dev) {
        LOG(ERROR) << "couldn't find device.";
        return false;
    }

    libusb_config_descriptor* config;
    if (libusb_get_active_config_descriptor(dev, &config) < 0) {
        LOG(ERROR) << "failed to get active config descriptor";
        return false;
    }

#if 0
    cout << "# of interfaces = " << (int)config->bNumInterfaces << endl;
    for (int i = 0; i < config->bNumInterfaces; ++i) {
        cout << "interface " << i << endl;
        const libusb_interface* interface = config->interface + i;
        cout << "  # of settings = " << interface->num_altsetting << endl;
        for (int j = 0; j < interface->num_altsetting; ++j) {
            const libusb_interface_descriptor* setting = interface->altsetting + j;
            cout << "  setting " << j << endl;
            cout << "    # of endpoints = " << (int)setting->bNumEndpoints << endl;
            for (int k = 0; k < setting->bNumEndpoints; ++k) {
                cout << "      endpoint " << k << endl;
                const libusb_endpoint_descriptor* endpoint = setting->endpoint + k;
                cout << "        descType = " << (int)endpoint->bDescriptorType << endl;
                cout << "        attribute = " << (int)endpoint->bmAttributes << endl;
                cout << "        address = " << (int)endpoint->bEndpointAddress << endl;
            }
        }
    }
#endif

    if (libusb_open(dev, &deviceHandle_) < 0 || !deviceHandle_) {
        LOG(ERROR) << "Failed to open USB device";
        return false;
    }
    libusb_unref_device(dev);

    // TODO(mayah): Don't use signal, but use sigaction.
    signal(SIGTERM, SyntekDriver::release);
    int ret = libusb_claim_interface(deviceHandle_, 0);
    if (ret < 0) {
        if (ret == LIBUSB_ERROR_BUSY) {
            LOG(ERROR) << "Failed to claim device interface: already running?";
        } else {
            LOG(ERROR) << "Failed to claim device interface";
        }
        return false;
    }

    if (libusb_set_interface_alt_setting(deviceHandle_, 0, 0) < 0) {
        LOG(ERROR) << "Failed to set active alternate setting for interface";
        return false;
    }

    return true;
}

bool SyntekDriver::initDevice()
{
    if (!writeIndex(0x0500, 0x0094))
        return false;
    if (!writeIndex(0x0203, 0x00a0))
        return false;

    // (void)[dev setFeatureAtIndex:1];

    if (!writeIndex(0x0003, 0x0080))
        return false;
    if (!writeIndex(0x0001, 0x0003))
        return false;
    if (!writeIndex(0x0000, 0x0021))
        return false;
    if (!writeIndex(0x0002, 0x0067))
        return false;

    struct {
        uint16_t reg;
        uint16_t val;
    } static const settings[] = {
        {0x203, 0x04a},
        {0x00d, 0x000},
        {0x00f, 0x002},
        {0x103, 0x000},
        {0x018, 0x000},
        {0x01a, 0x014},
        {0x01b, 0x00e},
        {0x01c, 0x046},
        {0x019, 0x000},
        {0x300, 0x012},
        {0x350, 0x02d},
        {0x351, 0x001},
        {0x352, 0x000},
        {0x353, 0x000},
        {0x300, 0x080},
        {0x018, 0x010},
        {0x202, 0x00f},
    };
    for (size_t i = 0; i < ARRAY_SIZE(settings); ++i) {
        if (!writeIndex(settings[i].reg, settings[i].val))
            return false;
    }
    if (!writeIndex(0x00100, 0x0033))
        return false;
    return true;
}

bool SyntekDriver::initChip()
{
    uint8_t modeSource = SAA711XMODESVideoAI12_YGain;
    uint8_t field = SAA711XFSELManualFieldSelection60Hz;
    uint8_t format = SAA711XCSTDNTSCM;
    uint8_t polarity = SAA711XRTP0OutputPolarityInverted;
    uint8_t output = SAA711XCH1ENAD1X | SAA711XCH2ENAD2X;

    // Based on Table 184 in the datasheet.
    struct {
        uint8_t reg;
        int16_t val;
    } const settings[] = {
        {0x01, 0x08},
        {0x02, static_cast<int16_t>(SAA711XFUSE0Antialias | SAA711XFUSE1Amplifier | modeSource)},
        {0x03, static_cast<int16_t>(SAA711XHOLDGAutomaticGainControlEnabled | SAA711XVBSLLongVerticalBlanking)},
        {0x04, 0x90},
        {0x05, 0x90},
        {0x06, 0xeb},
        {0x07, 0xe0},
        {0x08, static_cast<int16_t>(SAA711XVNOIVerticalNoiseReductionFast | SAA711XHTCHorizontalTimeConstantFastLocking | SAA711XFOETForcedOddEventToggle | field)},
        {0x09, 0x0000 }, // Uh, what was this?
        {0x0e, static_cast<int16_t>(SAA711XCCOMBAdaptiveChrominanceComb | SAA711XFCTCFastColorTimeConstant | format)},
        {0x0f, static_cast<int16_t>(SAA711XCGAINChromaGainValueNominal | SAA711XACGCAutomaticChromaGainControlEnabled)},
        {0x10, 0x06},
        {0x11, polarity},
        {0x12, 0x00},
        {0x13, 0x00},
        {0x14, 0x01},
        {0x15, 0x11},
        {0x16, 0xfe},
        {0x17, 0x00}, // Must be 0x00 for GM7113 (v10).
        {0x18, 0x40},
        {0x19, 0x80},
        {0x1a, 0x77},
        {0x1b, 0x42},
        {0x1c, 0xa9},
        {0x1d, 0x01},
        {0x83, 0x31},
        {0x88, static_cast<int16_t>(SAA711XSLM1ScalerDisabled | SAA711XSLM3AudioClockGenerationDisabled | output)},
    };

    for (size_t i = 0; i < ARRAY_SIZE(settings); ++i) {
        if (!writeSAA711XRegister(settings[i].reg, settings[i].val))
            return false;
    }
    for (int i = 0x41; i <= 0x57; ++i) {
        if (!writeSAA711XRegister(i, 0xff))
            return false;
    }

    // brightness
    if (!writeSAA711XRegister(0x0a, 0x5f))
        return false;
    // contrast
    if (!writeSAA711XRegister(0x0b, 0x40))
        return false;
    // saturation
    if (!writeSAA711XRegister(0x0c, 0x40))
        return false;
    // hue
    if (!writeSAA711XRegister(0x0d, 00))
        return false;

    return true;

}

void SyntekDriver::logDriverVersion()
{
    uint8_t version = 0;
    if (!readSAA711XRegister(0x00, &version)) {
        LOG(ERROR) << "failed to read version";
    }

    LOG(INFO) << "syntek version=" << (int)version;
}

bool SyntekDriver::initAudio()
{
    if (!writeVT1612ARegister(0x94, 0x00))
        return false;
    if (!writeIndex(0x0506, 0x0001))
        return false;
    if (!writeIndex(0x0507, 0x0000))
        return false;
    if (!initializeVT1612AChip())
        return false;

    return true;
}

bool SyntekDriver::initResolution()
{
    const int inputSizeWidth = 720;
    const int inputSizeHeight = 240 * 2; // NTSC
    const int standardSizeWidth = 720;
    const int standardSizeHeight = 240 * 2 + 6;
    const size_t bpp = 2;
    const uint16_t is50Hz = 0;

    struct {
        uint16_t reg;
        uint16_t val;
    } settings[] = {
        {0x110, (standardSizeWidth - inputSizeWidth) * bpp},
        {0x111, 0},
        {0x112, (standardSizeHeight - inputSizeHeight) / 2},
        {0x113, 0},
        {0x114, standardSizeWidth * bpp},
        {0x115, 5},
        {0x116, standardSizeHeight / 2},
        {0x117, is50Hz},
    };

    for (size_t i = 0; i < ARRAY_SIZE(settings); i++) {
        if (!writeIndex(settings[i].reg, settings[i].val))
            return false;
    }
    return true;
}

bool SyntekDriver::setAlternateInterface(uint8_t alternateSetting)
{
    int result = libusb_set_interface_alt_setting(deviceHandle_, 0, alternateSetting);
    if (result < 0) {
        LOG(ERROR) << "failed to setAlternateInterface";
        return false;
    }

    return true;
}

bool SyntekDriver::setStreaming(bool flag)
{
    uint8_t value;
    if (!readIndex(STK0408StatusRegistryIndex, &value))
        return false;

    if (flag) {
        value |= STK0408StatusStreaming;
    } else {
        value &= ~STK0408StatusStreaming;
    }

    return writeIndex(STK0408StatusRegistryIndex, value);
}

bool SyntekDriver::readIndex(int index, uint8_t* out)
{
    VLOG(1) << "read: index=0x" << index;

    uint8_t v = 0;
    int result = libusb_control_transfer(
        deviceHandle_,
        LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
        LIBUSB_REQUEST_GET_STATUS,
        0,
        index,
        &v,
        sizeof(v),
        500);

    if (result < 0) {
        LOG(ERROR) << "failed to readIndex";
        return false;
    }

    if (out)
        *out = v;
    return result;
}

bool SyntekDriver::writeIndex(int index, int value)
{
    int result = libusb_control_transfer(
        deviceHandle_,
        LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
        LIBUSB_REQUEST_CLEAR_FEATURE,
        value,
        index,
        nullptr,
        0,
        500);

    if (result < 0) {
        LOG(ERROR) << "failed to writeIndex";
        return false;
    }

    return true;
}

bool SyntekDriver::readSAA711XRegister(uint8_t reg, uint8_t* outVal)
{
    if (!writeIndex(0x208, reg))
        return false;
    if (!writeIndex(0x200, 0x20))
        return false;
    if (!SAA711XExpect(0x01)) {
        LOG(ERROR) << "SAA711X failed to read " << reg;
        return false;
    }

    return readIndex(0x209, outVal);
}

bool SyntekDriver::writeSAA711XRegister(uint8_t reg, int16_t val)
{
    if (!writeIndex(0x0204, reg))
        return false;
    if (!writeIndex(0x0205, val))
        return false;
    if (!writeIndex(0x0200, 0x0001))
        return false;

    if (!SAA711XExpect(0x04)) {
        LOG(ERROR) << "SAA711X failed to write " << val << " to " << reg;
        return false;
    }

    return true;
}

bool SyntekDriver::SAA711XExpect(uint8_t val)
{
    int retry = 4;
    uint8_t result = 0;
    while (retry--) {
        if (!readIndex(0x201, &result))
            return false;
        if (val == result)
            return true;
        usleep(100);
    }

    LOG(ERROR) << "Invalid SAA711X result: " << result << " expected=" << val;
    return false;
}

inline uint16_t VT1612ATwoChannels(uint8_t left, uint8_t right)
{
    return left | (static_cast<uint16_t>(right) << 8);
}
static uint16_t VT1612ABothChannels(uint8_t v)
{
    return VT1612ATwoChannels(v, v);
}
static uint8_t VT1612AInputGain(double v)
{
    return clamp(0x0, static_cast<int>(std::round((1.0f - v) * 0x1f)), 0x1f);
}
static uint8_t VT1612ARecordGain(double v)
{
    return clamp(0x0, static_cast<int>(std::round(v * 0x0f)), 0x15);
}

bool SyntekDriver::initializeVT1612AChip()
{
    struct {
        uint8_t reg;
        uint16_t val;
    } settings[] = {
        {VT1612ARegisterRecordSelect, VT1612ABothChannels(VT1612ARecordSourceLineIn)},
        {VT1612ARegisterVolumeLineIn, VT1612ABothChannels(VT1612AInputGain(1.0))},
        {VT1612ARegisterRecordGain, VT1612ABothChannels(VT1612ARecordGain(0.3))},
    };

    for (size_t i = 0; i < ARRAY_SIZE(settings); i++) {
        writeVT1612ARegister(settings[i].reg, settings[i].val);
    }

    return true;
}

bool SyntekDriver::writeVT1612ARegister(uint8_t reg, uint16_t val)
{
    if (!writeIndex(0x0504, reg))
        return false;
    if (!writeIndex(0x0502, (val & 0xFF)))
        return false;
    if (!writeIndex(0x0503, (val >> 8) & 0xFF))
        return false;
    if (!writeIndex(0x0500, 0x8c))
        return false;
    return true;
}

// static
void SyntekDriver::dataReceivedCallback(libusb_transfer* transfer)
{
    SyntekDriver* driver = static_cast<SyntekDriver*>(transfer->user_data);
    driver->dataReceived(transfer);
}

void SyntekDriver::dataReceived(libusb_transfer* transfer)
{
    VLOG(1) << "data received: " << transfer->num_iso_packets;

    pendingRequests_--;
    CHECK(pendingRequests_ >= 0) << pendingRequests_;

    int n = transfer->num_iso_packets;
    for (int i = 0; i < n; ++i) {
        unsigned char* data = libusb_get_iso_packet_buffer_simple(transfer, i);
        int length = transfer->iso_packet_desc[i].actual_length;
        if (length <= 0)
            continue;

        int skip = (data[0] & ECVSTK1160NewImageFlag) ? 8 : 4;
        const bool isHigh = (data[0] & ECVSTK1160HighFieldFlag);
        int* const offset = isHigh ? &highOffset_ : &lowOffset_;
        unsigned char* const buffer = isHigh ? highBuffer_ : lowBuffer_;

        if (data[0] & ECVSTK1160NewImageFlag) {
            *offset = 0;
            if (isHigh && imageReceivedCallback_) {
                const int bytesPerRow = 720 * 2;
                imageReceivedCallback_(highBuffer_, lowBuffer_, bytesPerRow, 240);
            }
        }

        if (length <= skip)
            continue;
        for (int i = skip; i < length && *offset < BUFFER_SIZE; ++i) {
            buffer[(*offset)++] = data[i];
        }

    }

    int ret = libusb_submit_transfer(transfer);
    if (ret < 0) {
        LOG(ERROR) << "failed to submit transfere";
        return;
    }
    pendingRequests_++;
}

void SyntekDriver::runRead()
{
    const int numberOfTransfers = 32;
    const int frameRequestSize = 3 * 1024;
    const int numIsoPackets = 32;

    libusb_transfer** transfers = new libusb_transfer*[numberOfTransfers];
    unsigned char** isobuf = new unsigned char*[numberOfTransfers];
    for (int i = 0; i < numberOfTransfers; ++i) {
        transfers[i] = libusb_alloc_transfer(numIsoPackets); // ?
        isobuf[i] = new unsigned char[64 * 3072];
        // TODO(mayah) 0x82=130, should be taken from endpoint_descriptor.
        libusb_fill_iso_transfer(transfers[i], deviceHandle_, 0x82, isobuf[i], 64 * 3072, numIsoPackets, dataReceivedCallback, this, 2000);
        libusb_set_iso_packet_lengths(transfers[i], frameRequestSize);
    }

    for (int i = 0; i < numberOfTransfers; ++i) {
        ++pendingRequests_;
        int ret = libusb_submit_transfer(transfers[i]);
        if (ret < 0) {
            LOG(ERROR) << "failed to submit transfer " << i << " because of " << ret;
        }
    }

    while (pendingRequests_ > 0) {
        libusb_handle_events(nullptr);
    }

    for (int i = 0; i < numberOfTransfers; ++i) {
        libusb_free_transfer(transfers[i]);
        delete[] isobuf[i];
    }
    delete[] transfers;
    delete[] isobuf;
}
