#include "source.h"

#include <errno.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>

#include <SDL_image.h>

DEFINE_string(save_img_dir, "/tmp", "");

using namespace std;

static void saveImg(SDL_Surface* surf, const char* prefix, int ss_num)
{
    char buf[256];
    {
        sprintf(buf, "%s/%d", FLAGS_save_img_dir.c_str(), ss_num / 10000);
        if (mkdir(buf, 0755) < 0) {
            if (errno != EEXIST) {
                LOG(ERROR) << "failed to mkdir: path=" << buf;
            }
        }
    }

    sprintf(buf, "%s/%d/%s-puyo%07d.bmp", FLAGS_save_img_dir.c_str(), ss_num / 10000, prefix, ss_num);
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

    return surface;
}
