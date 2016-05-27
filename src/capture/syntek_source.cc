#include "capture/syntek_source.h"

#include <memory>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "capture/driver/syntek.h"

#include <iostream>

using namespace std;

DEFINE_int32(initial_discards, 5, "Discards several initial frames.");
DEFINE_int32(capture_offset_x, 37, "The x offset to crop captured image.");
DEFINE_int32(capture_offset_y, 8, "The y offset to crop captured image.");
DEFINE_int32(capture_width, 640, "The cropped captured image width.");
DEFINE_int32(capture_height, 224, "The cropped captured image height.");

SyntekSource::SyntekSource() :
    discarded_(0)
{
    width_ = 320;
    height_ = 224;
    ok_ = false;

    driver_ = SyntekDriver::open();
    if (driver_) {
        ok_ = true;
    }
}

SyntekSource::~SyntekSource()
{

}

bool SyntekSource::start()
{
    if (!ok())
        return false;

    auto callback = [this](const unsigned char* buffer,
                           bool /*isHigh*/,
                           int bytesPerRow,
                           int numRowsPerBuffer) {
        lock_guard<mutex> lock(mu_);

        if (discarded_ < FLAGS_initial_discards) {
            ++discarded_;
            return;
        }

        UniqueSDLSurface surf(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 720, 240, 32, 0, 0, 0, 0)));

        int* pixels = static_cast<int*>(surf->pixels);

        // Copy to the current pixel data.
        int pos = 0;
        for (int y = 0; y < numRowsPerBuffer; ++y) {
            for (int i = 0; i < bytesPerRow; i += 4) {
                int u = buffer[0], y1 = buffer[1], v = buffer[2], y2 = buffer[3];
                int r, g, b;
                convertUVY2RGBA(u, v, y1, &r, &g, &b);
                pixels[pos++] = b + (g << 8) + (r << 16);
                convertUVY2RGBA(u, v, y2, &r, &g, &b);
                pixels[pos++] = b + (g << 8) + (r << 16);
                buffer += 4;
            }
        }

        surfaces_queue_.push(std::move(surf));
        // cond_.notify_one();
    };

    driver_->setImageReceivedCallback(callback);
    th_ = thread([this]() {
        runLoop();
    });
    th_.detach();
    return true;
}

UniqueSDLSurface SyntekSource::getNextFrame()
{
    while (surfaces_queue_.size() >= 2) {
        (void)surfaces_queue_.take();
    }

    UniqueSDLSurface raw_surf = surfaces_queue_.take();
    UniqueSDLSurface surf(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 320, 224, 32, 0, 0, 0, 0)));
    // Convert 720x240 to 640x224.
    const SDL_Rect srcRect {
        FLAGS_capture_offset_x, FLAGS_capture_offset_y,
        FLAGS_capture_width, FLAGS_capture_height };
    SDL_BlitScaled(raw_surf.get(), &srcRect, surf.get(), nullptr);
    return surf;
}

void SyntekSource::runLoop()
{
    driver_->runRead();
}
