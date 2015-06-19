#ifndef GUI_BOX_H_
#define GUI_BOX_H_

#include <SDL.h>

struct Box {
    Box() : Box(0, 0, 0, 0) {}
    Box(int sx, int sy, int dx, int dy) : sx(sx), sy(sy), dx(dx), dy(dy) {}

    int w() const { return dx - sx; }
    int h() const { return dy - sy; }

    SDL_Rect toSDLRect() const { return SDL_Rect { sx, sy, w(), h() }; }

    void moveOffset(int x, int y)
    {
        sx += x;
        dx += x;
        sy += y;
        dy += y;
    }

    Box shrink(int d) const { return Box(sx + d, sy + d, dx - d, dy - d); }

    int sx, sy, dx, dy;
  };

#endif
