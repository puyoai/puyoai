#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <SDL.h>
#include <SDL_image.h>

#include "capture/capture.h"
#include "capture/color.h"
#include "capture/ac_analyzer.h"
#include "gui/bounding_box.h"
#include "gui/main_window.h"
#include "gui/unique_sdl_surface.h"
#include "gui/SDL_prims.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <in-bmp> <out-up-bmp> <out-down-bmp>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(argv[1])));
    if (!surf) {
        fprintf(stderr, "Failed to load %s!\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // TODO(mayah): Since bounding box is initialized in ACAnalyzer, we need to use this here.
    // This must be wrong.
    ACAnalyzer analyzer;

    const int IMAGE_OFFSET_Y = 80 - 32 - 32;
    const int IMAGE_OFFSET_X = 71 - 32;
    const int IMAGE_WIDTH = 32 * 20;
    // const int IMAGE_HEIGHT = 32 * 14;

    UniqueSDLSurface up(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 320, 224, 32, 0, 0, 0, 0)));
    UniqueSDLSurface down(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 320, 224, 32, 0, 0, 0, 0)));

    // up
    for (int i = 0; i < 224; ++i) {
        const int y = IMAGE_OFFSET_Y + 2 * i;
        SDL_Rect srcRect { IMAGE_OFFSET_X, y, IMAGE_WIDTH, 1 };
        SDL_Rect dstRect { 0, i, 320, 1 };
        SDL_BlitScaled(surf.get(), &srcRect, up.get(), &dstRect);
    }

    for (int i = 0; i < 224; ++i) {
        const int y = IMAGE_OFFSET_Y + 2 * i + 1;
        SDL_Rect srcRect { IMAGE_OFFSET_X, y, IMAGE_WIDTH, 1 };
        SDL_Rect dstRect { 0, i, 320, 1 };
        SDL_BlitScaled(surf.get(), &srcRect, down.get(), &dstRect);
    }

    SDL_SaveBMP(up.get(), argv[2]);
    SDL_SaveBMP(down.get(), argv[3]);

    return 0;
}
