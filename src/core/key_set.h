#ifndef CORE_KEY_SET_H_
#define CORE_KEY_SET_H_

#include "core/key.h"

#include <bitset>
#include <string>
#include <vector>

struct KeySet {
    KeySet() {}
    explicit KeySet(Key);
    KeySet(Key, Key);

    static std::string toDebugString(const std::vector<KeySet>&);

    void setKey(Key, bool b = true);
    bool hasKey(Key) const;

    bool hasSomeKey() const { return keys_.any(); }
    bool hasIntersection(const KeySet& rhs) const {
        const KeySet& lhs = *this;
        return (lhs.keys_ & rhs.keys_).any();
    }

    std::string toString() const;
    int toInt() const { return static_cast<int>(keys_.to_ulong()); }

    friend bool operator==(const KeySet& lhs, const KeySet& rhs) {
        return lhs.keys_ == rhs.keys_;
    }

private:
    std::bitset<NUM_KEYS> keys_;
};

#endif
