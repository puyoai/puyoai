#include "movie.h"
#include "capture.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

DEFINE_int32(fps, 0, "");
DEFINE_bool(save_all_frames, false, "");

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <in-movie>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  SDL_Init(SDL_INIT_VIDEO);

  Movie::init();
  Movie* movie = new Movie(argv[1]);

  if (!movie->ok()) {
    fprintf(stderr, "Failed to load %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  Capture cap(movie->width(), movie->height(), 24);

  bool stop = false;
  while (1) {
    Uint32 start_ticks = SDL_GetTicks();
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
      case SDL_QUIT:
        return 0;
      case SDL_KEYDOWN: {
        if (ev.key.keysym.sym == SDLK_SPACE) {
          stop = !stop;
        }
      }
      }
    }
    if (stop) {
      SDL_Delay(100);
      continue;
    }

    SDL_Surface* surf = movie->getNextFrame();
    if (!surf)
      break;
    cap.addFrame(surf);
    cap.show();

    if (FLAGS_save_all_frames) {
      Source::saveScreenShot(surf, NULL);
    }

    Uint32 elapsed = SDL_GetTicks() - start_ticks;
    if (FLAGS_fps) {
      int d = (1000 / FLAGS_fps) - elapsed;
      if (d > 0)
        SDL_Delay(d);
    }
  }

  SDL_Quit();

  return 0;
}
