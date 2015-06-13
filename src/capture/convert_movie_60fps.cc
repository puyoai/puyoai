#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <SDL.h>
#include <SDL_image.h>

#include "capture/ac_analyzer.h"
#include "capture/capture.h"
#include "capture/color.h"
#include "capture/movie_source.h"
#include "gui/bounding_box.h"
#include "gui/main_window.h"
#include "gui/unique_sdl_surface.h"
#include "gui/SDL_prims.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <in-bmp>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    MovieSource::init();
    MovieSource source(argv[1]);
    if (!source.ok()) {
        cerr << "failed to load " << argv[1] << endl;
        return EXIT_FAILURE;
    }

    source.setFPS(0);
    int num = 0;

    // TODO(mayah): Since bounding box is initialized in ACAnalyzer, we need to use this here.
    // This must be wrong.
    ACAnalyzer analyzer;

    const int IMAGE_OFFSET_Y = 80 - 32 - 32;
    const int IMAGE_OFFSET_X = 70 - 32;
    const int IMAGE_WIDTH = 32 * 20;
    // const int IMAGE_HEIGHT = 32 * 14;

    while (true) {
        source.nextStep();
        UniqueSDLSurface surf = source.getNextFrame();
        if (!surf.get())
            break;

        UniqueSDLSurface up(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 320, 224, 32, 0, 0, 0, 0)));
        UniqueSDLSurface down(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 320, 224, 32, 0, 0, 0, 0)));

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

        char buf[80];
        sprintf(buf, "orig-%05d.bmp", ++num);
        SDL_SaveBMP(up.get(), buf);
        sprintf(buf, "orig-%05d.bmp", ++num);
        SDL_SaveBMP(down.get(), buf);
    }

    return 0;
}
