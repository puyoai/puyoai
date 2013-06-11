#ifndef CORE_KUMIPUYO_H_
#define CORE_KUMIPUYO_H_

#include <string>
#include "core/puyo_color.h"

class Kumipuyo {
public:
    Kumipuyo() : axis(PuyoColor::EMPTY), child(PuyoColor::EMPTY) {}
    Kumipuyo(PuyoColor axis, PuyoColor child) : axis(axis), child(child) {}

    std::string toString() const {
        char tmp[] = "  ";
        tmp[0] = charOfPuyoColor(axis);
        tmp[1] = charOfPuyoColor(child);

        return tmp;
    }

    bool isValid() const {
        return isNormalColor(axis) && isNormalColor(child);
    }

public:
    PuyoColor axis;
    PuyoColor child;
};

class KumipuyoPos {
public:
    static const KumipuyoPos& InitialPos();

    KumipuyoPos() : x(0), y(0), r(0) {}
    KumipuyoPos(int x, int y, int r) : x(x), y(y), r(r) {}
    //KumipuyoPos(const Decision& d);

    std::string debugString() const;

public:
    int x;
    int y;
    int r;
};

inline const KumipuyoPos& KumipuyoPos::InitialPos()
{
    static const KumipuyoPos initial_pos(3, 12, 0);
    return initial_pos;
}

class KumipuyoSeq {
public:
    KumipuyoSeq() {}
    KumipuyoSeq(const std::string&);
    KumipuyoSeq(const std::vector<Kumipuyo>&);
    KumipuyoSeq(std::initializer_list<Kumipuyo>);

    const Kumipuyo& get(int n) const { return seq_[n]; }
    PuyoColor axis(int n) const { return seq_[n].axis; }
    PuyoColor child(int n) const { return seq_[n].child; }

    bool isEmpty() const { return seq_.empty(); }
    int size() const { return seq_.size(); }
    void resize(int n) { seq_.resize(n); }

    const Kumipuyo& front() const { return seq_.front(); }
    void dropFront() { seq_.erase(seq_.begin()); }

    KumipuyoSeq subsequence(int begin, int n) const {
        KumipuyoSeq seq;
        seq.seq_.assign(seq_.begin() + begin, seq_.begin() + begin + n);
        return seq;
    }

    std::string toString() const;

    const std::vector<Kumipuyo>& underlyingData() const { return seq_; }

private:
    std::vector<Kumipuyo> seq_;
};

#endif
