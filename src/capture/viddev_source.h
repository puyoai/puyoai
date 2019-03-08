#ifndef CAPTURE_VIDDEV_SOURCE_H_
#define CAPTURE_VIDDEV_SOURCE_H_

#ifndef USE_V4L2
# error "USE_V4L2 must be defined to include viddev_source.h"
#endif

#include <string>

#include <SDL.h>

#include "capture/source.h"

class VidDevSource : public Source {
public:
    explicit VidDevSource(const std::string& dev);
    virtual ~VidDevSource();

    virtual UniqueSDLSurface getNextFrame() override;

private:
    struct Buffer {
        char* start;
        size_t length;
        SDL_Surface* surface;
    };

    void init();
    void initBuffers();

    void quit();

    const std::string dev_;
    int fd_;
    // TODO(peria): Use C++ STL container to manage buffers.
    Buffer* buffers_;
    size_t buf_cnt_;
};

#endif  // CAPTURE_VIDDEV_H_
