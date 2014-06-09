#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <SDL.h>
#include <SDL_image.h>

#include "capture/capture.h"
#include "capture/images.h"
#include "gui/main_window.h"
#include "gui/unique_sdl_surface.h"

using namespace std;

DEFINE_bool(verbose, false, "");

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s <in-img>...\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  SDL_Init(SDL_INIT_VIDEO);

  UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(argv[1])));

  MainWindow mainWindow;
  Capture cap(&mainWindow);
#if 0
  Capture cap(surf->w, surf->h, 16);
#endif

  vector<string> image_filenames(argv + 1, argv + argc);
  Images images(image_filenames);

  images.stop();
  while (!images.done()) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_m) {
        printf("%s\n", cap.getMessageFor(0).c_str());
        continue;
      }

      images.handleEvent(ev);
    }

    images.handleKeys();

    if (images.isStopped() && !images.isIndexUpdated()) {
      SDL_Delay(10);
      continue;
    }

    UniqueSDLSurface surf(images.getNextFrame());
    assert(surf.get());

    cap.setFrameInfo(images.getCurrentFilename());
    cap.addSourceFrame(std::move(surf));
    cap.show();

    if (FLAGS_verbose) {
      printf("%s\n", images.getCurrentFilename().c_str());
      printf("%s\n", cap.getMessageFor(0).c_str());
    }
  }

  SDL_Quit();

  return 0;
}
