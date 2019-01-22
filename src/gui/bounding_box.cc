#include "gui/bounding_box.h"

#include <glog/logging.h>

namespace {
#if 1
const int MOVIE_SCALE = 2; // 入力の動画サイズとウィンドウサイズの比

const double W = 21.5;
const double H = 20;

const int RIGHT_FIELD_X = 92;
const int RIGHT_FIELD_Y = 52;
#else
const int W = 32;
const int H = 32;
#endif
}

// static
Box BoundingBox::boxForDraw(int pi, int x, int y)
{
    x = 12 * pi + x;
#if 1
    int sx = static_cast<int>(RIGHT_FIELD_X + (x - 1) * W);
    int dx = static_cast<int>(RIGHT_FIELD_X + (x) * W);
    int sy = static_cast<int>(RIGHT_FIELD_Y + (12 - y) * H);
    int dy = static_cast<int>(RIGHT_FIELD_Y + (13 - y) * H);
#else
    int sx = static_cast<int>(x * W);
    int dx = static_cast<int>((x + 1) * W);
    int sy = static_cast<int>((13 - y) * H);
    int dy = static_cast<int>((14 - y) * H);
#endif

    return Box(sx, sy, dx, dy);
}

// static
Box BoundingBox::boxForDraw(int pi, NextPuyoPosition np)
{
    if (pi == 0) {
        switch (np) {
        case NextPuyoPosition::CURRENT_AXIS:
        case NextPuyoPosition::CURRENT_CHILD:
            return Box(0, 0, 0, 0);
        case NextPuyoPosition::NEXT1_AXIS: {
            Box b = boxForDraw(0, 8, 9);
            b.moveOffset(-1, 0);
            return b;
        }
        case NextPuyoPosition::NEXT1_CHILD: {
            Box b =  boxForDraw(0, 8, 10);
            b.moveOffset(-1, 0);
            return b;
        }
        case NextPuyoPosition::NEXT2_AXIS: {
            Box b = boxForDraw(0, 9, 8);
            b.dx = (b.sx + b.dx) / 2;
            return b;
        }
        case NextPuyoPosition::NEXT2_CHILD: {
            Box b = boxForDraw(0, 9, 9);
            b.dx = (b.sx + b.dx) / 2;
            return b;
        }
        }
    } else if (pi == 1) {
        switch (np) {
        case NextPuyoPosition::CURRENT_AXIS:
        case NextPuyoPosition::CURRENT_CHILD:
            return Box(0, 0, 0, 0);
        case NextPuyoPosition::NEXT1_AXIS: {
            Box b = boxForDraw(1, -1, 9);
            b.moveOffset(1, 0);
            return b;
        }
        case NextPuyoPosition::NEXT1_CHILD: {
            Box b = boxForDraw(1, -1, 10);
            b.moveOffset(1, 0);
            return b;
        }
        case NextPuyoPosition::NEXT2_AXIS: {
            Box b = boxForDraw(1, -2, 8);
            b.sx = (b.sx + b.dx) / 2;
            return b;
        }
        case NextPuyoPosition::NEXT2_CHILD: {
            Box b = boxForDraw(1, -2, 9);
            b.sx = (b.sx + b.dx) / 2;
            return b;
        }
        }
    }

    CHECK(false) << "Unknown player id: pi = " << pi;
    return Box(0, 0, 0, 0);
}

// static
Box BoundingBox::boxForDraw(Region region)
{
    switch (region) {
    case Region::LEVEL_SELECT_1P: return Box(224, 240, 240, 272);
    case Region::LEVEL_SELECT_2P: return Box(400, 240, 416, 272);
    case Region::GAME_FINISHED:   return Box(255, 336, 383, 350);
    default: return Box(0, 0, 0, 0);
    }
}

// static
Box BoundingBox::boxForAnalysis(int pi, int x, int y)
{
    Box b = boxForDraw(pi, x, y);
#if 1
    return Box(b.sx * MOVIE_SCALE, b.sy * MOVIE_SCALE, b.dx * MOVIE_SCALE, b.dy * MOVIE_SCALE);
#else
    return Box(b.sx / 2, b.sy / 2, b.dx / 2, b.dy / 2);
#endif
}

Box BoundingBox::boxForAnalysis(int pi, NextPuyoPosition np)
{
    Box b = boxForDraw(pi, np);
#if 1
    return Box(b.sx * MOVIE_SCALE, b.sy * MOVIE_SCALE, b.dx * MOVIE_SCALE, b.dy * MOVIE_SCALE);
#else
    return Box(b.sx / 2, b.sy / 2, b.dx / 2, b.dy / 2);
#endif
}

Box BoundingBox::boxForAnalysis(Region region)
{
    Box b = boxForDraw(region);
#if 1
    return Box(b.sx * MOVIE_SCALE, b.sy * MOVIE_SCALE, b.dx * MOVIE_SCALE, b.dy * MOVIE_SCALE);
#else
    return Box(b.sx / 2, b.sy / 2, b.dx / 2, b.dy / 2);
#endif
}
