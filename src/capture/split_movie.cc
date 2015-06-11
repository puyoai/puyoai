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
        fprintf(stderr, "Usage: %s <movie>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    MovieSource::init();

    MovieSource source(argv[1]);
    if (!source.ok()) {
        fprintf(stderr, "Failed to load %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    source.setFPS(0);

    // TODO(mayah): Since bounding box is initialized in ACAnalyzer, we need to use this here.
    // This must be wrong.
    ACAnalyzer analyzer;

    int num = 0;

    while (true) {
        source.nextStep();
        UniqueSDLSurface surf = source.getNextFrame();
        if (!surf.get())
            break;

        const int WIDTH = 32;
        const int HEIGHT = 32;

        for (int pi = 0; pi < 2; ++pi) {
            for (int y = 12; y >= 1; --y) {
                for (int x = 1; x <= 6; ++x) {
                    Box b = BoundingBox::instance().get(pi, x, y);
                    BoxAnalyzeResult result = analyzer.analyzeBox(surf.get(), b);

                    const SDL_Rect rect = b.toSDLRect();
                    UniqueSDLSurface dest(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0)));
                    SDL_BlitSurface(surf.get(), &rect, dest.get(), nullptr);

                    char prefix;
                    switch (result.realColor) {
                    case RealColor::RC_EMPTY:  prefix = 'E'; break;
                    case RealColor::RC_WALL:   prefix = 'W'; break;
                    case RealColor::RC_OJAMA:  prefix = 'O'; break;
                    case RealColor::RC_RED:    prefix = 'R'; break;
                    case RealColor::RC_BLUE:   prefix = 'B'; break;
                    case RealColor::RC_YELLOW: prefix = 'Y'; break;
                    case RealColor::RC_GREEN:  prefix = 'G'; break;
                    case RealColor::RC_PURPLE: prefix = 'P'; break;
                    default: CHECK(false) << "Unknown color";
                    }

                    char filename[80];
                    sprintf(filename, "%c%06d.bmp", prefix, ++num);
                    SDL_SaveBMP(dest.get(), filename);

                    cout << toChar(result.realColor);
                }
                cout << endl;
            }
        }
    }

    return 0;
}
