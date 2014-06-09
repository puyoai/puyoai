#include "images.h"

#include <assert.h>
#include <stdlib.h>

#include <gflags/gflags.h>

#include <SDL_image.h>

using namespace std;

DEFINE_int32(index, 0, "");

Images::Images(const vector<string>& imgs)
  : images_(imgs),
    index_(FLAGS_index),
    prev_index_(-1),
    cur_(NULL),
    stop_(false) {
  assert(!imgs.empty());
  if (index_ >= (int)images_.size()) {
    fprintf(stderr, "Usage: --index=%d is too big\n", index_);
    exit(EXIT_FAILURE);
  }

  memset(key_state_, 0, sizeof(key_state_));

  SDL_Surface* surf = IMG_Load(imgs[0].c_str());
  if (!surf) {
    fprintf(stderr, "Failed to load %s\n", imgs[0].c_str());
    exit(EXIT_FAILURE);
  }
  width_ = surf->w;
  height_ = surf->h;
  SDL_FreeSurface(surf);

  ok_ = true;
}

Images::~Images() {
}

SDL_Surface* Images::getNextFrame() {
  if (cur_)
    SDL_FreeSurface(cur_);
  cur_ = NULL;

  prev_index_ = index_;
  cur_ = IMG_Load(images_[index_].c_str());
  if (cur_ == NULL) {
    fprintf(stderr, "Failed to load image: %s\n", images_[index_].c_str());
    abort();
  }
  if (!stop_)
    index_++;
  return cur_;
}

void Images::addIndex(int i) {
  index_ += i;
  if (index_ < 0)
    index_ = 0;
  if (index_ >= (int)images_.size())
    index_ = (int)images_.size() - 1;
}

const string& Images::getCurrentFilename() const {
  return images_[prev_index_];
}

void Images::handleEvent(const SDL_Event& ev) {
  switch (ev.type) {
  case SDL_QUIT:
    done_ = true;
    return;
  case SDL_KEYDOWN: {
    switch (ev.key.keysym.sym) {
    case SDLK_s:
      stop_ = !stop_;
      break;
    case SDLK_f:
      printf("%s\n", getCurrentFilename().c_str());
      break;
    case SDLK_ESCAPE:
      done_ = true;
      return;
    default:
      break;
    }
  }
  }
}

void Images::handleKeys() {
  Uint8* keys = SDL_GetKeyState(NULL);
  for (int i = 0; i < SDLK_LAST; i++) {
    if (keys[i]) {
      if (key_state_[i] >= 1)
        key_state_[i]++;
      else
        key_state_[i] = 1;
    } else {
      key_state_[i] = 0;
    }
  }

  if (key_state_[SDLK_RIGHT] == 1 || key_state_[SDLK_RIGHT] > 10) {
    addIndex(1);
  }
  if (key_state_[SDLK_LEFT] == 1 || key_state_[SDLK_LEFT] > 10) {
    addIndex(-1);
  }
}
