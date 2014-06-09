#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <SDL.h>
#include <SDL_image.h>

#include "capture/capture.h"
#include "capture/color.h"
#include "capture/somagic_analyzer.h"
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

    UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(argv[1])));
    if (!surf) {
        fprintf(stderr, "Failed to load %s!\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    SomagicAnalyzer analyzer;

    // this should be called after |analyzer| is created.
#if 0
    Box b = BoundingBox::instance().get(0, NextPuyoPosition::NEXT2_AXIS);
#else
    Box b = BoundingBox::instance().get(0, 1, 4);
#endif
    analyzer.analyzeBox(surf.get(), b, true);

    analyzer.drawWithAnalysisResult(surf.get());
    SDL_SaveBMP(surf.get(), "output.bmp");

    return 0;
}
