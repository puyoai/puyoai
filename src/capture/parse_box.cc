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

void showUsage(const char* name)
{
    fprintf(stderr, "Usage: %s <in-bmp> [1|2] [NEXT1-AXIS|NEXT1-AXIS-HALF|NEXT1-CHILD|NEXT2-AXIS|NEXT2-CHILD]\n", name);
    fprintf(stderr, "Usage: %s <in-bmp> [1|2] x y\n", name);
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc < 4) {
        showUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    string imgFilename = argv[1];
    int playerId = std::atoi(argv[2]);

    bool half = false;
    bool usesNextPuyoPosition = false;
    NextPuyoPosition npp;
    int x, y;
    if (argv[3] == string("NEXT1-AXIS")) {
        usesNextPuyoPosition = true;
        npp = NextPuyoPosition::NEXT1_AXIS;
    } else if (argv[3] == string("NEXT1-AXIS-HALF")) {
        usesNextPuyoPosition = true;
        npp = NextPuyoPosition::NEXT1_AXIS;
        half = true;
    } else if (argv[3] == string("NEXT1-CHILD")) {
        usesNextPuyoPosition = true;
        npp = NextPuyoPosition::NEXT1_CHILD;
    } else if (argv[3] == string("NEXT2-AXIS")) {
        usesNextPuyoPosition = true;
        npp = NextPuyoPosition::NEXT2_AXIS;
    } else if (argv[3] == string("NEXT2-CHILD")) {
        usesNextPuyoPosition = true;
        npp = NextPuyoPosition::NEXT2_CHILD;
    } else {
        if (argc < 5) {
            showUsage(argv[0]);
            exit(EXIT_FAILURE);
        }

        x = atoi(argv[3]);
        y = atoi(argv[4]);
    }

    UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(imgFilename.c_str())));
    if (!surf) {
        fprintf(stderr, "Failed to load %s!\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    SomagicAnalyzer analyzer;

    // this should be called after |analyzer| is created.
    Box b;
    if (usesNextPuyoPosition) {
        b = BoundingBox::instance().get(playerId - 1, npp);
    } else {
        b = BoundingBox::instance().get(playerId - 1, x, y);
    }
    if (half) {
        b.sy += b.h() / 2;
    }

    BoxAnalyzeResult r = analyzer.analyzeBox(surf.get(), b, true);
    cout << "Color: " << toString(r.realColor) << endl;
    cout << "Vanishing: " << r.vanishing << endl;

    analyzer.drawWithAnalysisResult(surf.get());
    SDL_SaveBMP(surf.get(), "output.bmp");

    return 0;
}
