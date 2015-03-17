#ifndef CORE_KUMIPUYO_SEQ_H_
#define CORE_KUMIPUYO_SEQ_H_

#include <initializer_list>
#include <string>
#include <vector>

#include "core/kumipuyo.h"
#include "core/next_puyo.h"
#include "core/puyo_color.h"

class KumipuyoSeq {
public:
    KumipuyoSeq() {}
    KumipuyoSeq(const std::string&);
    KumipuyoSeq(const std::vector<Kumipuyo>&);
    KumipuyoSeq(std::initializer_list<Kumipuyo>);

    void clear() { seq_.clear(); }

    const Kumipuyo& get(int n) const { return seq_[n]; }
    PuyoColor color(NextPuyoPosition) const;
    PuyoColor axis(int n) const { return seq_[n].axis; }
    PuyoColor child(int n) const { return seq_[n].child; }

    bool isEmpty() const { return seq_.empty(); }
    int size() const { return seq_.size(); }
    void resize(int n) { seq_.resize(n); }

    const Kumipuyo& front() const { return seq_.front(); }
    void dropFront() { seq_.erase(seq_.begin()); }
    void add(const Kumipuyo& kp) { seq_.push_back(kp); }
    void append(const KumipuyoSeq& seq) { seq_.insert(seq_.end(), seq.seq_.begin(), seq.seq_.end()); }

    std::vector<Kumipuyo>::iterator begin() { return seq_.begin(); }
    std::vector<Kumipuyo>::const_iterator begin() const { return seq_.cbegin(); }
    std::vector<Kumipuyo>::iterator end() { return seq_.end(); }
    std::vector<Kumipuyo>::const_iterator end() const { return seq_.cend(); }

    KumipuyoSeq subsequence(int begin, int n) const;
    KumipuyoSeq subsequence(int begin) const;

    void setAxis(int n, PuyoColor c) { seq_[n].axis = c; }
    void setChild(int n, PuyoColor c) { seq_[n].child = c; }

    std::string toString() const;

    friend bool operator==(const KumipuyoSeq& lhs, const KumipuyoSeq& rhs) { return lhs.seq_ == rhs.seq_; }
    friend bool operator!=(const KumipuyoSeq& lhs, const KumipuyoSeq& rhs) { return !(lhs == rhs); }

private:
    std::vector<Kumipuyo> seq_;
};

#endif

