#include "gui/bounding_box.h"

#include <glog/logging.h>

namespace {
const int W = 32;
const int H = 32;
}

// static
Box BoundingBox::boxForDraw(int pi, int x, int y)
{
    x = 12 * pi + x;

    int sx = static_cast<int>(x * W);
    int dx = static_cast<int>((x + 1) * W);
    int sy = static_cast<int>((13 - y) * H);
    int dy = static_cast<int>((14 - y) * H);

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
        case NextPuyoPosition::NEXT1_AXIS:
            return boxForDraw(0, 8, 9);
        case NextPuyoPosition::NEXT1_CHILD:
            return boxForDraw(0, 8, 10);
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
        case NextPuyoPosition::NEXT1_AXIS:
            return boxForDraw(1, -1, 9);
        case NextPuyoPosition::NEXT1_CHILD:
            return boxForDraw(1, -1, 10);
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
    case Region::LEVEL_SELECT_1P: return Box(223, 256, 233, 280);
    case Region::LEVEL_SELECT_2P: return Box(405, 256, 415, 280);
    case Region::GAME_FINISHED:   return Box(255, 352, 383, 367);
    default: return Box(0, 0, 0, 0);
    }
}

// static
Box BoundingBox::boxForAnalysis(int pi, int x, int y)
{
    Box b = boxForDraw(pi, x, y);
    return Box(b.sx / 2, b.sy / 2, b.dx / 2, b.dy / 2);
}

Box BoundingBox::boxForAnalysis(int pi, NextPuyoPosition np)
{
    Box b = boxForDraw(pi, np);
    return Box(b.sx / 2, b.sy / 2, b.dx / 2, b.dy / 2);
}

Box BoundingBox::boxForAnalysis(Region region)
{
    Box b = boxForDraw(region);
    return Box(b.sx / 2, b.sy / 2, b.dx / 2, b.dy / 2);
}
