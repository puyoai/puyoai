#include "screen.h"

#include <assert.h>
#include <libgen.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#ifdef __CYGWIN__
#include <Windows.h>
#endif

#include <iostream>
#include <sstream>
#include <string>

#include <glog/logging.h>

#include "base/base.h"
#include "gui/util.h"

using namespace std;

Screen::Screen(int width, int height, const Box& mainBox) :
    surface_(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000))),
    mainBox_(mainBox)
{
    init();
}

void Screen::init()
{
    bgColor_.r = 198;
    bgColor_.g = 196;
    bgColor_.b = 156;

    if (!TTF_WasInit()) {
        CHECK_EQ(TTF_Init(), 0) << TTF_GetError();
    }

    char buf[PATH_MAX+1];
#ifdef __CYGWIN__
    if (!GetCurrentDirectory(PATH_MAX, buf))
        PLOG(FATAL) << "buffer is too small for getcwd";
#else
    if (!getcwd(buf, PATH_MAX))
        PLOG(FATAL) << "buffer is too small for getcwd";
#endif

    char* p = buf;
    while (true) {
        string font_filename = string(p) + "/data/mikachan-p.ttf";
        if (access(font_filename.c_str(), R_OK) == 0) {
            font_ = TTF_OpenFont(font_filename.c_str(), 16);
            if (!font_) {
                LOG(FATAL) << TTF_GetError();
            }
            break;
        }

        p = dirname(p);
        if (!p[1]) {
            LOG(FATAL) << "Font not found";
        }
    }
}

Screen::~Screen()
{
    TTF_CloseFont(font_);
}

void Screen::clear()
{
    SDL_FillRect(surface(), NULL, SDL_MapRGB(surface()->format, bgColor_.r, bgColor_.g, bgColor_.b));
}
