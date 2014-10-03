#ifndef CORE_KUMIPUYO_H_
#define CORE_KUMIPUYO_H_

#include <string>

#include "core/puyo_color.h"
#include "core/next_puyo.h"

class Kumipuyo {
public:
    constexpr Kumipuyo() : axis(PuyoColor::EMPTY), child(PuyoColor::EMPTY) {}
    constexpr Kumipuyo(PuyoColor axis, PuyoColor child) : axis(axis), child(child) {}

    std::string toString() const;

    bool isValid() const { return isNormalColor(axis) && isNormalColor(child); }

    friend bool operator==(const Kumipuyo& lhs, const Kumipuyo& rhs) { return lhs.axis == rhs.axis && lhs.child == rhs.child; }
    friend bool operator!=(const Kumipuyo& lhs, const Kumipuyo& rhs) { return !(lhs == rhs); }

public:
    PuyoColor axis;
    PuyoColor child;
};

class KumipuyoSeq {
public:
    KumipuyoSeq() {}
    KumipuyoSeq(const std::string&);
    KumipuyoSeq(const std::vector<Kumipuyo>&);
    KumipuyoSeq(std::initializer_list<Kumipuyo>);

    const Kumipuyo& get(int n) const { return seq_[n]; }
    PuyoColor color(NextPuyoPosition) const;
    PuyoColor axis(int n) const { return seq_[n].axis; }
    PuyoColor child(int n) const { return seq_[n].child; }

    bool isEmpty() const { return seq_.empty(); }
    int size() const { return seq_.size(); }
    void resize(int n) { seq_.resize(n); }

    const Kumipuyo& front() const { return seq_.front(); }
    void dropFront() { seq_.erase(seq_.begin()); }
    void append(const KumipuyoSeq& seq) { seq_.insert(seq_.end(), seq.seq_.begin(), seq.seq_.end()); }

    std::vector<Kumipuyo>::iterator begin() { return seq_.begin(); }
    std::vector<Kumipuyo>::const_iterator begin() const { return seq_.cbegin(); }
    std::vector<Kumipuyo>::iterator end() { return seq_.end(); }
    std::vector<Kumipuyo>::const_iterator end() const { return seq_.cend(); }

    KumipuyoSeq subsequence(int begin, int n) const
    {
        KumipuyoSeq seq;
        seq.seq_.assign(seq_.begin() + begin, seq_.begin() + begin + n);
        return seq;
    }

    void setAxis(int n, PuyoColor c) { seq_[n].axis = c; }
    void setChild(int n, PuyoColor c) { seq_[n].child = c; }

    std::string toString() const;

    friend bool operator==(const KumipuyoSeq& lhs, const KumipuyoSeq& rhs) { return lhs.seq_ == rhs.seq_; }
    friend bool operator!=(const KumipuyoSeq& lhs, const KumipuyoSeq& rhs) { return !(lhs == rhs); }

private:
    std::vector<Kumipuyo> seq_;
};

#endif
