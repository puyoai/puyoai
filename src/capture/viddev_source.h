#ifndef CAPTURE_VIDDEV_H_
#define CAPTURE_VIDDEV_H_

#include <SDL.h>

#include "source.h"

class VidDevSource : public Source {
public:
    explicit VidDevSource(const char* dev);
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

    const char* dev_;
    int fd_;
    Buffer* buffers_;
    size_t buf_cnt_;
};

#endif  // CAPTURE_VIDDEV_H_
