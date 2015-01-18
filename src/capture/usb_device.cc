#include "capture/usb_device.h"

libusb_device *findDevice(int vendor, int product)
{
    libusb_device** list = nullptr;
    libusb_device* dev = nullptr;

    ssize_t count = libusb_get_device_list(NULL, &list);
    for (ssize_t i = 0; i < count; i++) {
        libusb_device* item = list[i];
        libusb_device_descriptor descriptor;
        libusb_get_device_descriptor(item, &descriptor);
        if (descriptor.idVendor == vendor && descriptor.idProduct == product) {
            dev = item;
        } else {
            libusb_unref_device(item);
        }
    }

    libusb_free_device_list(list, 0);
    return dev;
}
