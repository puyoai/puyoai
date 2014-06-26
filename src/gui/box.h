#ifndef GUI_BOX_H_
#define GUI_BOX_H_

#include <SDL.h>

struct Box {
    Box() : sx(0), sy(0), dx(0), dy(0) {}
    Box(int sx, int sy, int dx, int dy) : sx(sx), sy(sy), dx(dx), dy(dy) {}

    int w() const { return dx - sx; }
    int h() const { return dy - sy; }

    SDL_Rect toSDLRect() const {
        return SDL_Rect { sx, sy, w(), h() };
    }

    int sx, sy, dx, dy;
  };

#endif
