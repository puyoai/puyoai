#ifndef CAPTURE_MOVIE_SOURCE_H_
#define CAPTURE_MOVIE_SOURCE_H_

#include <stdint.h>

#include <atomic>
#include <string>

#include <SDL.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "capture/source.h"
#include "gui/unique_sdl_surface.h"

class MovieSource : public Source {
public:
    explicit MovieSource(const char* filename);
    explicit MovieSource(const std::string& filename) :
        MovieSource(filename.c_str()) {}

    virtual ~MovieSource();

    virtual UniqueSDLSurface getNextFrame();

    void setFPS(int fps) { fps_ = fps; }
    void nextStep();

    static void init();

private:
    const char* filename_;

    int fps_ = 60;
    Uint32 lastTaken_ = 0;
    // Default must be true to show the first image.
    std::atomic<bool> waitUntilTrue_;

    AVFormatContext* format_;
    AVCodecContext* codec_;
    AVFrame* frame_;
    AVFrame* frame_rgb_;
    int video_index_;

    AVPacket packet_;
    SwsContext* sws_;

    UniqueSDLSurface surf_;
};

#endif  // CAPTURE_MOVIE_H_
