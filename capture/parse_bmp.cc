#include <stdio.h>
#include <stdlib.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <SDL.h>
#include <SDL_image.h>

#include "capture.h"

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <in-bmp> <out-bmp>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Surface* surf = IMG_Load(argv[1]);
  if (!surf) {
    fprintf(stderr, "Failed to load %s!\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  Capture cap(surf->w, surf->h, 16);

  cap.addFrame(surf);
  printf("%llu ticks\n", cap.capture_ticks());
  cap.show();
  if (argv[2]) {
    cap.save(argv[2]);
  } else {
    while (true) {
      SDL_Event ev;
      while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
          return 0;
        case SDL_KEYDOWN: {
          switch (ev.key.keysym.sym) {
          case SDLK_ESCAPE:
            return 0;
          default:
            break;
          }
        }
        }
      }

      SDL_Delay(100);
    }
  }

  return 0;
}
