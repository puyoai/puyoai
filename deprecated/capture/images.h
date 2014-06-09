#ifndef CAPTURE_IMAGES_H_
#define CAPTURE_IMAGES_H_

#include <string>
#include <vector>

#include <SDL.h>

#include "source.h"

class Images : public Source {
 public:
  explicit Images(const std::vector<std::string>& imgs);
  virtual ~Images();

  virtual SDL_Surface* getNextFrame();
  virtual void handleEvent(const SDL_Event& ev);
  virtual void handleKeys();

  int index() const { return index_; }
  bool isIndexUpdated() const { return index_ != prev_index_; }
  void addIndex(int i);

  bool isStopped() const { return stop_; }
  void stop() { stop_ = true; }

  const std::string& getCurrentFilename() const;

 private:
  const std::vector<std::string> images_;
  int index_, prev_index_;
  SDL_Surface* cur_;
  bool stop_;
  int key_state_[SDLK_LAST];
};

#endif  // CAPTURE_IMAGES_H_
