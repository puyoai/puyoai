#include "core/kumipuyo.h"

using namespace std;

string Kumipuyo::toString() const
{
    char tmp[] = "  ";
    tmp[0] = toChar(axis);
    tmp[1] = toChar(child);

    return tmp;
}

