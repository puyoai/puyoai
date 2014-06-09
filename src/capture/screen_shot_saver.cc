#include "capture/screen_shot_saver.h"

#include <iostream>

#include <SDL.h>
#include <gflags/gflags.h>

#include "gui/screen.h"

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

void ScreenShotSaver::draw(Screen* screen)
{
    // Save only if the frame is different from the previous one.
    // userdata is set in Capture::draw().
    SDL_Surface* surface = screen->surface();
    if (!surface)
        return;

    if (drawsFrameId_) {
        SDL_Color color { 0, 0, 0, 255 };
        string text = "Frame: " + to_string(reinterpret_cast<uintptr_t>(surface->userdata));
        UniqueSDLSurface textSurface(makeUniqueSDLSurface(TTF_RenderUTF8_Blended(screen->font(), text.c_str(), color)));

        if (textSurface.get()) {
            SDL_Rect dr {
                static_cast<Sint16>(surface->w / 2 - textSurface->w / 2),
                static_cast<Sint16>(surface->h - textSurface->h - 25),
                0,
                0,
            };

            SDL_BlitSurface(textSurface.get(), nullptr, surface, &dr);
        }
    }

    if (reinterpret_cast<uintptr_t>(surface->userdata) != lastFrameId_) {
        lastFrameId_ = reinterpret_cast<uintptr_t>(surface->userdata);
        cout << lastFrameId_ << endl;
        saveScreenShot(surface);
    }
}
