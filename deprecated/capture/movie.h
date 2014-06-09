#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifndef CAPTURE_MOVIE_H_
#define CAPTURE_MOVIE_H_

#include <stdint.h>

#include <SDL.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "source.h"

class Movie : public Source {
 public:
  explicit Movie(const char* filename);

  virtual ~Movie();

  virtual SDL_Surface* getNextFrame();

  int width() const { return width_; }
  int height() const { return height_; }

  static void init();

 private:
  const char* filename_;
  AVFormatContext* format_;
  AVCodecContext* codec_;
  AVFrame* frame_;
  AVFrame* frame_rgb_;
  int video_index_;

  AVPacket packet_;
  SwsContext* sws_;

  SDL_Surface* surf_;
};

#endif  // CAPTURE_MOVIE_H_
