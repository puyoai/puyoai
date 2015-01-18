#include "gui/bounding_box.h"

#include <glog/logging.h>

// static
BoundingBox& BoundingBox::instance()
{
    static BoundingBox ins;
    return ins;
}

BoundingBox::BoundingBox() :
    offsetX_(0),
    offsetY_(0),
    bbWidth_(0),
    bbHeight_(0)
{
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::setGenerator(double offsetX, double offsetY, double bbWidth, double bbHeight)
{
    offsetX_ = offsetX;
    offsetY_ = offsetY;
    bbWidth_ = bbWidth;
    bbHeight_ = bbHeight;
}

Box BoundingBox::get(int pi, int x, int y) const
{
    x = 12 * pi + x;

    int sx = static_cast<int>(offsetX_ + (x - 1) * bbWidth_);
    int dx = static_cast<int>(offsetX_ + x * bbWidth_);
    int sy = static_cast<int>(offsetY_ + (11 - y) * bbHeight_);
    int dy = static_cast<int>(offsetY_ + (12 - y) * bbHeight_);

    return Box(sx, sy, dx, dy);
}

Box BoundingBox::get(int pi, NextPuyoPosition np) const
{
    if (pi == 0) {
        switch (np) {
        case NextPuyoPosition::CURRENT_AXIS:
        case NextPuyoPosition::CURRENT_CHILD:
            return Box(0, 0, 0, 0);
        case NextPuyoPosition::NEXT1_AXIS:
            return get(0, 8, 9);
        case NextPuyoPosition::NEXT1_CHILD:
            return get(0, 8, 10);
        case NextPuyoPosition::NEXT2_AXIS: {
            Box b = get(0, 9, 8);
            b.dx = (b.sx + b.dx) / 2;
            return b;
        }
        case NextPuyoPosition::NEXT2_CHILD: {
            Box b = get(0, 9, 9);
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
            return get(1, -1, 9);
        case NextPuyoPosition::NEXT1_CHILD:
            return get(1, -1, 10);
        case NextPuyoPosition::NEXT2_AXIS: {
            Box b = get(1, -2, 8);
            b.sx = (b.sx + b.dx) / 2;
            return b;
        }
        case NextPuyoPosition::NEXT2_CHILD: {
            Box b = get(1, -2, 9);
            b.sx = (b.sx + b.dx) / 2;
            return b;
        }
        }
    }

    CHECK(false) << "Unknown player id: pi = " << pi;
    return Box(0, 0, 0, 0);
}
