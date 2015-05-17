#ifndef CORE_KEY_SET_SEQ_H_
#define CORE_KEY_SET_SEQ_H_

#include <bitset>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>

#include <glog/logging.h>

#include "core/key.h"
#include "core/key_set.h"

class KeySetSeq {
public:
    KeySetSeq() {}
    explicit KeySetSeq(const std::vector<KeySet>& seq) : seq_(seq) {}
    explicit KeySetSeq(std::initializer_list<KeySet> seq) : seq_(seq) {}
    explicit KeySetSeq(const std::string&);

    bool empty() const { return seq_.empty(); }
    size_t size() const { return seq_.size(); }
    const KeySet& front() const { return seq_.front(); }
    const KeySet& back() const { return seq_.back(); }
    const KeySet& operator[](int idx) const { return seq_[idx]; }

    std::vector<KeySet>::iterator begin() { return seq_.begin(); }
    std::vector<KeySet>::const_iterator begin() const { return seq_.cbegin(); }
    std::vector<KeySet>::iterator end() { return seq_.end(); }
    std::vector<KeySet>::const_iterator end() const { return seq_.cend(); }

    void add(const KeySet& ks) { seq_.push_back(ks); }
    // TODO(mayah): This is not O(1) but O(N). However, this method won't be called much.
    // So it would be acceptable.
    void removeFront() { seq_.erase(seq_.begin()); }
    // TODO(mayah): This is O(N).
    void addFront(const KeySet& ks) { seq_.insert(seq_.begin(), ks); }

    void clear() { seq_.clear(); }

    std::string toString() const;

private:
    std::vector<KeySet> seq_;
};

class PrecedeKeySetSeq {
public:
    PrecedeKeySetSeq() {}
    explicit PrecedeKeySetSeq(const KeySetSeq& seq) : seq_(seq), precedeSeq_(seq) {}
    explicit PrecedeKeySetSeq(const std::string& seq) : seq_(seq), precedeSeq_(seq) {}
    explicit PrecedeKeySetSeq(const std::string& seq, char precede, const std::string& precedeSeq) :
        seq_(seq),
        precede_(precede == ' ' ? KeySet() : KeySet(toKey(precede))),
        precedeSeq_(precedeSeq)
    {
        DCHECK_EQ(seq_.empty(), precedeSeq_.empty()) << seq_.toString() << ' ' << precedeSeq_.toString();
    }

    // Key sequence without precede input.
    const KeySetSeq& seq() const { return seq_; }
    // Key sequence with precede input.
    const KeySet& precede() const { return precede_; }
    const KeySetSeq& precedeSeq() const { return precedeSeq_; }

    bool empty() const { return seq_.empty(); }

private:
    const KeySetSeq seq_;
    const KeySet precede_;
    const KeySetSeq precedeSeq_;
};

#endif
