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

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <in-bmp>\n", argv[0]);
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

    const int WIDTH = 32;
    const int HEIGHT = 32;

    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            Box b = BoundingBox::boxForAnalysis(0, x, y);
            RealColor rc = analyzer.analyzeBox(surf.get(), b);

            const SDL_Rect rect = b.toSDLRect();
            UniqueSDLSurface dest(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0)));
            SDL_BlitSurface(surf.get(), &rect, dest.get(), nullptr);

            ostringstream filename;
            switch (rc) {
            case RealColor::RC_EMPTY:  filename << 'E'; break;
            case RealColor::RC_WALL:   filename << 'W'; break;
            case RealColor::RC_OJAMA:  filename << 'O'; break;
            case RealColor::RC_RED:    filename << 'R'; break;
            case RealColor::RC_BLUE:   filename << 'B'; break;
            case RealColor::RC_YELLOW: filename << 'Y'; break;
            case RealColor::RC_GREEN:  filename << 'G'; break;
            case RealColor::RC_PURPLE: filename << 'P'; break;
            }
            filename << "-" << x << "-" << y << ".bmp";
            SDL_SaveBMP(dest.get(), filename.str().c_str());

            cout << toChar(rc);
        }
        cout << endl;
    }

    return 0;
}
