#include "capture/syntek_source.h"

#include <memory>
#include <glog/logging.h>

#include "capture/driver/syntek.h"

#include <iostream>

using namespace std;

SyntekSource::SyntekSource()
{
    width_ = 720;
    height_ = 480;
    ok_ = false;

    driver_ = SyntekDriver::open();
    if (driver_) {
        currentPixelData_.reset(new int[height() * width()]);
        ok_ = true;
    }
}

bool SyntekSource::start()
{
    if (!ok())
        return false;

    auto callback = [this](const unsigned char* higher,
                           const unsigned char* lower,
                           int bytesPerRow,
                           int numRowsPerBuffer) {
        lock_guard<mutex> lock(mu_);

        // Copy to the current pixel data.
        int pos = 0;
        for (int y = 0; y < numRowsPerBuffer; ++y) {
            // For higher
            for (int i = 0; i < bytesPerRow; i += 4) {
                int u = higher[0], y1 = higher[1], v = higher[2], y2 = higher[3];
                int r, g, b;
                convertUVY2RGBA(u, v, y1, &r, &g, &b);
                currentPixelData_[pos++] = b + (g << 8) + (r << 16);
                convertUVY2RGBA(u, v, y2, &r, &g, &b);
                currentPixelData_[pos++] = b + (g << 8) + (r << 16);
                higher += 4;
            }
            // For lower
            for (int i = 0; i < bytesPerRow; i += 4) {
                int u = lower[0], y1 = lower[1], v = lower[2], y2 = lower[3];
                int r, g, b;
                convertUVY2RGBA(u, v, y1, &r, &g, &b);
                currentPixelData_[pos++] = b + (g << 8) + (r << 16);
                convertUVY2RGBA(u, v, y2, &r, &g, &b);
                currentPixelData_[pos++] = b + (g << 8) + (r << 16);
                lower += 4;
            }
        }

        cond_.notify_one();
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
    if (!currentPixelData_)
        return makeUniqueSDLSurface(nullptr);

    SDL_Surface* surface = SDL_CreateRGBSurface(0, width(), height(), 32, 0, 0, 0, 0);
    unique_lock<mutex> lock(mu_);
    cond_.wait(lock);

    CHECK_EQ(SDL_LockSurface(surface), 0);
    memmove(surface->pixels, currentPixelData_.get(), width() * height() * sizeof(int));
    SDL_UnlockSurface(surface);
    return makeUniqueSDLSurface(surface);
}

void SyntekSource::runLoop()
{
    driver_->runRead();
}
