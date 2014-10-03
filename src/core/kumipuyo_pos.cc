#include "core/kumipuyo_pos.h"

using namespace std;

string KumipuyoPos::toDebugString() const
{
    char buf[256];
    snprintf(buf, 256, "<y=%d,x=%d,r=%d>", y, x, r);
    return std::string(buf);
}
