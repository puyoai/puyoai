#include "core/puyo_color.h"

static_assert(sizeof(PuyoColor) == 1, "PuyoColor should be 1 byte");

PuyoColor toPuyoColor(char c)
{
    switch (c) {
    case '0':
    case ' ':
        return PuyoColor::EMPTY;
    case '1':
    case 'O': // not zero but Oh
        return PuyoColor::OJAMA;
    case '2':
        return PuyoColor::WALL;
    case '4':
    case 'R':
    case 'r':
        return PuyoColor::RED;
    case '5':
    case 'B':
    case 'b':
        return PuyoColor::BLUE;
    case '6':
    case 'Y':
    case 'y':
        return PuyoColor::YELLOW;
    case '7':
    case 'G':
    case 'g':
        return PuyoColor::GREEN;
    default:
        ;
    }

    // Unfortunately, some code uses 0 <= c <= 7.
    // TODO(mayah): Remove this code once all codes are safe!
    if (0 <= c && c <= static_cast<int>(PuyoColor::GREEN)) {
        return static_cast<PuyoColor>(c);
    }

    // No match.
    return PuyoColor::EMPTY;
}

char charOfPuyoColor(PuyoColor c, char charIfEmpty)
{
    switch (c) {
    case PuyoColor::EMPTY: return charIfEmpty;
    case PuyoColor::OJAMA: return '@';
    case PuyoColor::WALL: return '#';
    case PuyoColor::RED: return 'R';
    case PuyoColor::BLUE: return 'B';
    case PuyoColor::YELLOW: return 'Y';
    case PuyoColor::GREEN: return 'G';
    }

    return '?';
}

std::string toString(PuyoColor c)
{
    switch (c) {
    case PuyoColor::EMPTY: return "";
    case PuyoColor::OJAMA: return "";
    case PuyoColor::WALL: return "";
    case PuyoColor::RED: return "赤";
    case PuyoColor::BLUE: return "青";
    case PuyoColor::YELLOW: return "黄";
    case PuyoColor::GREEN: return "緑";
    }

    return "";
}

