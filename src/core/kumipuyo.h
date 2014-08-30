#ifndef CORE_KUMIPUYO_H_
#define CORE_KUMIPUYO_H_

#include <string>
#include "core/puyo_color.h"

class Kumipuyo {
public:
    Kumipuyo() : axis(PuyoColor::EMPTY), child(PuyoColor::EMPTY) {}
    Kumipuyo(PuyoColor axis, PuyoColor child) : axis(axis), child(child) {}

    std::string toString() const
    {
        char tmp[] = "  ";
        tmp[0] = toChar(axis);
        tmp[1] = toChar(child);

        return tmp;
    }

    bool isValid() const
    {
        return isNormalColor(axis) && isNormalColor(child);
    }

    friend bool operator==(const Kumipuyo& lhs, const Kumipuyo& rhs)
    {
        return lhs.axis == rhs.axis && lhs.child == rhs.child;
    }
    friend bool operator!=(const Kumipuyo& lhs, const Kumipuyo& rhs) { return !(lhs == rhs); }

public:
    PuyoColor axis;
    PuyoColor child;
};

class KumipuyoPos {
public:
    static const KumipuyoPos& InitialPos();

    KumipuyoPos() : x(0), y(0), r(0) {}
    KumipuyoPos(int x, int y, int r) : x(x), y(y), r(r) {}

    int axisX() const { return x; }
    int axisY() const { return y; }
    int childX() const { return x + (r == 1) - (r == 3); }
    int childY() const { return y + (r == 0) - (r == 2); }
    int rot() const { return r; }

    std::string toDebugString() const;

    friend bool operator==(const KumipuyoPos& lhs, const KumipuyoPos& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.r == rhs.r;
    }
    friend bool operator!=(const KumipuyoPos& lhs, const KumipuyoPos& rhs) { return !(lhs == rhs); }

public:
    // TODO(mayah): Make these private?
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

    void setAxis(int n, PuyoColor c) { seq_[n].axis = c; }
    void setChild(int n, PuyoColor c) { seq_[n].child = c; }

    std::string toString() const;

    const std::vector<Kumipuyo>& underlyingData() const { return seq_; }
    friend bool operator==(const KumipuyoSeq& lhs, const KumipuyoSeq& rhs) { return lhs.seq_ == rhs.seq_; }
    friend bool operator!=(const KumipuyoSeq& lhs, const KumipuyoSeq& rhs) { return !(lhs == rhs); }

private:
    std::vector<Kumipuyo> seq_;
};

#endif
