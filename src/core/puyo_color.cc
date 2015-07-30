#include "core/puyo_color.h"

static_assert(sizeof(PuyoColor) == 1, "PuyoColor should be 1 byte");

PuyoColor toPuyoColor(char c)
{
    switch (c) {
    case '0':
    case ' ':
    case '.':
        return PuyoColor::EMPTY;
    case '1':
    case 'O': // not zero but Oh
    case '@':
        return PuyoColor::OJAMA;
    case '2':
    case '#':
        return PuyoColor::WALL;
    case '3':
    case '&':
        return PuyoColor::IRON;
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
        // No match.
        return PuyoColor::EMPTY;
    }
}

char toChar(PuyoColor c, char charIfEmpty)
{
    switch (c) {
    case PuyoColor::EMPTY: return charIfEmpty;
    case PuyoColor::OJAMA: return '@';
    case PuyoColor::WALL: return '#';
    case PuyoColor::IRON: return '&';
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
    case PuyoColor::IRON: return "IRON";
    case PuyoColor::RED: return "RED";
    case PuyoColor::BLUE: return "BLUE";
    case PuyoColor::YELLOW: return "YELLOW";
    case PuyoColor::GREEN: return "GREEN";
    }

    return "";
}
