#ifndef CAPTURE_VIDDEV_H_
#define CAPTURE_VIDDEV_H_

#include <SDL.h>

#include "source.h"

class VidDev : public Source {
 public:
  explicit VidDev(const char* dev);
  virtual ~VidDev();

  virtual SDL_Surface* getNextFrame();

 private:
  struct Buffer {
    char* start;
    size_t length;
    SDL_Surface* surface;
  };

  void initBuffers();

  int fd_;
  Buffer* buffers_;
};

#endif  // CAPTURE_VIDDEV_H_
