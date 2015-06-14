#include "capture/syntek_source.h"

#include <memory>
#include <glog/logging.h>

#include "capture/driver/syntek.h"

#include <iostream>

using namespace std;

SyntekSource::SyntekSource() :
    currentSurface_(emptyUniqueSDLSurface())
{
    width_ = 640;
    height_ = 448;
    ok_ = false;

    driver_ = SyntekDriver::open();
    if (driver_) {
        currentSurface_.reset(SDL_CreateRGBSurface(0, 720, 240, 32, 0, 0, 0, 0));
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
        CHECK_EQ(SDL_LockSurface(currentSurface_.get()), 0);

        int* pixels = static_cast<int*>(currentSurface_->pixels);

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

        SDL_UnlockSurface(currentSurface_.get());
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
    if (!currentSurface_.get())
        return makeUniqueSDLSurface(nullptr);

    unique_lock<mutex> lock(mu_);
    cond_.wait(lock);

    UniqueSDLSurface surf(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 640, 224, 32, 0, 0, 0, 0)));
    CHECK_EQ(SDL_LockSurface(currentSurface_.get()), 0);
    // 720x240 -> 640x224
    const SDL_Rect srcRect { 40, 8, 640, 224 };
    SDL_BlitScaled(currentSurface_.get(), &srcRect, surf.get(), nullptr);
    SDL_UnlockSurface(currentSurface_.get());

    return std::move(surf);
}

void SyntekSource::runLoop()
{
    driver_->runRead();
}
