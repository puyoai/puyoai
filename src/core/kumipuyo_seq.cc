#include "core/kumipuyo_seq.h"

#include <glog/logging.h>

using namespace std;

KumipuyoSeq::KumipuyoSeq(const string& str)
{
    for (string::size_type i = 0; i * 2 + 1 < str.size(); ++i) {
        PuyoColor axis = toPuyoColor(str[i * 2]);
        PuyoColor child = toPuyoColor(str[i * 2 + 1]);
        seq_.push_back(Kumipuyo(axis, child));
    }
}

KumipuyoSeq::KumipuyoSeq(const vector<Kumipuyo>& seq) : seq_(seq)
{
}

KumipuyoSeq::KumipuyoSeq(initializer_list<Kumipuyo> seq) : seq_(seq)
{
}

PuyoColor KumipuyoSeq::color(NextPuyoPosition npp) const
{
    switch (npp) {
    case NextPuyoPosition::CURRENT_AXIS:
        return size() > 0 ? axis(0) : PuyoColor::EMPTY;
    case NextPuyoPosition::CURRENT_CHILD:
        return size() > 0 ? child(0) : PuyoColor::EMPTY;
    case NextPuyoPosition::NEXT1_AXIS:
        return size() > 1 ? axis(1) : PuyoColor::EMPTY;
    case NextPuyoPosition::NEXT1_CHILD:
        return size() > 1 ? child(1) : PuyoColor::EMPTY;
    case NextPuyoPosition::NEXT2_AXIS:
        return size() > 2 ? axis(2) : PuyoColor::EMPTY;
    case NextPuyoPosition::NEXT2_CHILD:
        return size() > 2 ? child(2) : PuyoColor::EMPTY;
    }

    CHECK(false) << "Unknown NextPuyoPosition: " << static_cast<int>(npp);
    return PuyoColor::EMPTY;
}

string KumipuyoSeq::toString() const
{
    std::string s;
    for (const auto& kumipuyo : seq_) {
        s += toChar(kumipuyo.axis);
        s += toChar(kumipuyo.child);
    }

    return s;
}

KumipuyoSeq KumipuyoSeq::subsequence(int begin, int n) const
{
    DCHECK_LE(begin, static_cast<int>(seq_.size()));
    DCHECK_LE(begin + n, static_cast<int>(seq_.size()));
    DCHECK_LE(0, n);

    KumipuyoSeq seq;
    seq.seq_.assign(seq_.begin() + begin, seq_.begin() + begin + n);
    return seq;
}

KumipuyoSeq KumipuyoSeq::subsequence(int begin) const
{
    DCHECK_LE(begin, static_cast<int>(seq_.size()));

    KumipuyoSeq seq;
    seq.seq_.assign(seq_.begin() + begin, seq_.end());
    return seq;
}
