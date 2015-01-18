#ifndef CAPTURE_USB_DEVICE_H_
#define CAPTURE_USB_DEVICE_H_

#include <libusb-1.0/libusb.h>

// Finds device by |vendor| and |product|.
libusb_device* findDevice(int vendor, int product);

#endif
