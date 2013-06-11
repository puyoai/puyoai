#include "core/kumipuyo.h"

using namespace std;

#if 0
KumipuyoPos::KumipuyoPos(const Decision& d)
{
    if (d.r == 3) {
        this->x = d.x + 1;
    } else {
        this->x = d.x;
    }
    this->r = d.r;
    this->y = 2;
}
#endif

std::string KumipuyoPos::debugString() const
{
    char buf[256];
    snprintf(buf, 256, "<y=%d,x=%d,r=%d>", y, x, r);
    return std::string(buf);
}

KumipuyoSeq::KumipuyoSeq(const string& str)
{
    for (string::size_type i = 0; i * 2 + 1 < str.size(); ++i) {
        PuyoColor axis = puyoColorOf(str[i * 2]);
        PuyoColor child = puyoColorOf(str[i * 2 + 1]);
        seq_.push_back(Kumipuyo(axis, child));
    }
}

KumipuyoSeq::KumipuyoSeq(const vector<Kumipuyo>& seq) : seq_(seq)
{
}

KumipuyoSeq::KumipuyoSeq(initializer_list<Kumipuyo> seq) : seq_(seq)
{
}

string KumipuyoSeq::toString() const
{
    std::string s;
    for (const auto& kumipuyo : seq_) {
        s += charOfPuyoColor(kumipuyo.axis);
        s += charOfPuyoColor(kumipuyo.child);
    }

    return s;
}

