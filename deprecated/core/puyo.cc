#include "puyo.h"

PuyoColor puyoColorOf(char c)
{
    switch (c) {
    case '0':
    case ' ':
        return EMPTY;
    case '1':
    case 'O': // not zero but Oh
        return OJAMA;
    case '2':
        return WALL;
    case '4':
    case 'R':
    case 'r':
        return RED;
    case '5':
    case 'B':
    case 'b':
        return BLUE;
    case '6':
    case 'Y':
    case 'y':
        return YELLOW;
    case '7':
    case 'G':
    case 'g':
        return GREEN;
    default:
        return EMPTY;
    }
}

char charOfPuyoColor(PuyoColor c)
{
    switch (c) {
    case EMPTY: return ' ';
    case OJAMA: return '@';
    case WALL: return '#';
    case RED: return 'R';
    case BLUE: return 'B';
    case YELLOW: return 'Y';
    case GREEN: return 'G';
    }

    return '?';
}

