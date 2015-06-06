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
#include "gui/bounding_box.h"
#include "gui/main_window.h"
#include "gui/unique_sdl_surface.h"
#include "gui/SDL_prims.h"

DECLARE_string(testdata_dir);

using namespace std;

// this program makes test patterns from foreground.png and background.png.

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    const pair<RealColor, int> colors[] = {
        make_pair(RealColor::RC_RED, 23),
        make_pair(RealColor::RC_BLUE, 21),
        make_pair(RealColor::RC_YELLOW, 23),
        make_pair(RealColor::RC_GREEN, 22),
        make_pair(RealColor::RC_PURPLE, 25),
        make_pair(RealColor::RC_OJAMA, 3)
    };

    UniqueSDLSurface foreground(makeUniqueSDLSurface(IMG_Load((FLAGS_testdata_dir + "/image/foreground.png").c_str())));
    UniqueSDLSurface background(makeUniqueSDLSurface(IMG_Load((FLAGS_testdata_dir + "/image/background.png").c_str())));

    for (int i = 0; i < 6; ++i) {
        UniqueSDLSurface surf(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 32 * 7, 32 * 3 * colors[i].second, 32, 0, 0, 0, 0)));
        for (int j = 0; j < colors[i].second; ++j) {
            SDL_Rect rect { 0, 32 * 3 * j, 32 * 7, 32 * 3};
            SDL_BlitSurface(background.get(), nullptr, surf.get(), &rect);

            SDL_Rect srcRect { j * 32, i * 32, 32, 32 };
            for (int x = 0; x < 7; ++x) {
                for (int y = 0; y < 3; ++y) {
                    SDL_Rect dstRect { 32 * x, 32 * 3 * j + 32 * y, 32, 32 };
                    SDL_BlitSurface(foreground.get(), &srcRect, surf.get(), &dstRect);
                }
            }
        }

        char buf[80];
        sprintf(buf, "%d.bmp", i);
        SDL_SaveBMP(surf.get(), buf);
    }

    return 0;
}
