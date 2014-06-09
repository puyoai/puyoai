#ifndef GUI_SCREEN_H_
#define GUI_SCREEN_H_

#include <map>
#include <string>

#include <SDL.h>
#include <SDL_ttf.h>

#include "core/puyo_color.h"
#include "gui/bounding_box.h"
#include "gui/box.h"
#include "gui/color_map.h"
#include "gui/unique_sdl_surface.h"

using namespace std;

class Screen {
public:
    Screen(int width, int height, const Box& mainBox);
    ~Screen();

    const Box& mainBox() { return mainBox_; }

    SDL_Surface* surface() { return surface_.get(); }

    SDL_Color bgColor() const { return bgColor_; }
    TTF_Font* font() { return font_; }

    void clear();

private:
    void init();

    UniqueSDLSurface surface_;
    Box mainBox_;

    SDL_Color bgColor_;
    TTF_Font* font_;
};

#endif  // DUEL_SCREEN_H_
