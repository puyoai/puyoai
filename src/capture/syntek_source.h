#ifndef CAPTURE_SYNTEK_SOURCE_H_
#define CAPTURE_SYNTEK_SOURCE_H_

#include <algorithm>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <libusb-1.0/libusb.h>

#include "base/base.h"
#include "base/blocking_queue.h"
#include "capture/capture_source.h"
#include "capture/source.h"
#include "gui/unique_sdl_surface.h"

class SyntekDriver;
class Screen;

class SyntekSource : public Source {
public:
    SyntekSource();
    virtual ~SyntekSource();

    virtual UniqueSDLSurface getNextFrame() override;
    virtual bool start() override;

private:
    void runLoop();

    std::thread th_;
    std::mutex mu_;

    base::InfiniteBlockingQueue<UniqueSDLSurface> surfaces_queue_;

    int discarded_;

    SyntekDriver* driver_;
};

#endif
