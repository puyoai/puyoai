#include "source.h"

#include <gflags/gflags.h>

#include <SDL_image.h>

DEFINE_string(save_img_dir, "/tmp", "");

using namespace std;

static void saveImg(SDL_Surface* surf, const char* prefix, int ss_num)
{
    char buf[256];
    sprintf(buf, "%s/%s-puyo%05d.bmp", FLAGS_save_img_dir.c_str(), prefix, ss_num);
    SDL_SaveBMP(surf, buf);
}

static void saveScreenShot(SDL_Surface* surf)
{
    static int ss_num = 0;
    saveImg(surf, "orig", ss_num);
    ss_num++;
}

Source::Source() :
    ok_(false),
    done_(false),
    width_(-1),
    height_(-1)
{
}

UniqueSDLSurface Source::nextFrame()
{
    UniqueSDLSurface surface = getNextFrame();

    if (savesScreenShot_ && surface.get()) {
        saveScreenShot(surface.get());
    }

    return std::move(surface);
}
