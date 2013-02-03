#include "capture.h"
#include "viddev.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <viddev>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

#ifdef __linux__

  SDL_Init(SDL_INIT_VIDEO);

  VidDev* dev = new VidDev(argv[1]);

  if (!dev->ok()) {
    fprintf(stderr, "Failed to load %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  Capture cap(dev->width(), dev->height(), 16);
  SDL_Surface* scr = cap.getScreen()->scr();

  Uint32 start_ticks = SDL_GetTicks();
  bool save_all_frames = false;
  while (1) {
    SDL_Surface* surf = dev->getNextFrame();
    if (!surf)
      break;

    cap.addFrame(surf);
    cap.show();

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
      case SDL_QUIT:
        return 0;
      case SDL_KEYDOWN:
        if (ev.key.keysym.sym == SDLK_s) {
          Source::saveScreenShot(surf, scr);
        } else if (ev.key.keysym.sym == SDLK_t) {
          save_all_frames = !save_all_frames;
        }
        break;
      }
    }

    if (save_all_frames) {
      Source::saveScreenShot(surf, scr);
    }

    if (0) {
      static int current_id = 0;
      current_id++;
      Uint32 now = SDL_GetTicks();
      if (now != start_ticks) {
        fprintf(stderr, "%f fps\n",
                (float)current_id * 1000 / (now - start_ticks));
      }
    }
  }

#endif

  return 0;
}
