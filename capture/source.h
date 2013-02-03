#ifndef CAPTURE_SOURCE_H_
#define CAPTURE_SOURCE_H_

#include <SDL.h>

class Source {
 public:
  virtual ~Source() {}

  virtual SDL_Surface* getNextFrame() = 0;

  virtual void handleEvent(const SDL_Event&) {}
  virtual void handleKeys() {}

  bool ok() const { return ok_; }

  int width() const { return width_; }
  int height() const { return height_; }

  static void saveScreenShot(SDL_Surface* surf, SDL_Surface* scr);

  bool done() const { return done_; }
  void end() { done_ = true; }

 protected:
  Source();

  bool ok_;
  int width_, height_;
  bool done_;
};

#endif  // CAPTURE_SOURCE_H_
