#ifndef CORE_PATTERN_PATTERN_BIT_H_
#define CORE_PATTERN_PATTERN_BIT_H_

#include <unordered_map>
#include <vector>

#include "core/field_bits.h"

class PatternBit {
public:
    PatternBit(FieldBits varBits, FieldBits notBits) : varBits_(varBits), notBits_(notBits) {}

    size_t hash() const { return varBits_.hash() * 37 + notBits_.hash(); }
    friend bool operator==(const PatternBit& lhs, const PatternBit& rhs)
    {
        return lhs.varBits_ == rhs.varBits_ && lhs.notBits_ == rhs.notBits_;
    }

private:
    FieldBits varBits_;
    FieldBits notBits_;
};

struct PatternBitHasher {
    size_t operator()(const PatternBit& pb) const { return pb.hash(); }
};

class PatternBitSet {
public:
    size_t size() const { return patternBits_.size(); }

    // Registers a new patter bit. Returns a PatternBit id.
    int put(const PatternBit&);

    const PatternBit& get(int id) const
    {
        DCHECK_LT(static_cast<size_t>(id), patternBits_.size());
        return patternBits_[id];
    }

private:
    std::vector<PatternBit> patternBits_;
    std::unordered_map<PatternBit, int, PatternBitHasher> registeredBits_;
};



#endif // CORE_PATTERN_PATTERN_BIT_H_
