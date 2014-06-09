#include "gui/puyo.h"

PuyoRealColor puyoRealColorOf(char c)
{
    switch (c) {
    case ' ': return PuyoRealColor::RC_EMPTY;
    case 'O': return PuyoRealColor::RC_OJAMA;
    case 'R': return PuyoRealColor::RC_RED;
    case 'B': return PuyoRealColor::RC_BLUE;
    case 'Y': return PuyoRealColor::RC_YELLOW;
    case 'G': return PuyoRealColor::RC_GREEN;
    case 'P': return PuyoRealColor::RC_PURPLE;
    }

    return RC_INVALID;
}

