#ifndef CORE_KEY_SET_H_
#define CORE_KEY_SET_H_

#include <bitset>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>

#include "core/key.h"

class KeySet {
public:
    KeySet() {}
    explicit KeySet(Key);
    KeySet(Key, Key);

    void setKey(Key, bool b = true);
    void setKey(char c);
    bool hasKey(Key) const;

    bool hasTurnKey() const { return hasKey(Key::RIGHT_TURN) || hasKey(Key::LEFT_TURN); }
    bool hasArrowKey() const { return hasKey(Key::RIGHT) || hasKey(Key::LEFT) || hasKey(Key::UP) || hasKey(Key::DOWN); }
    bool hasSomeKey() const { return keys_.any(); }
    bool hasIntersection(const KeySet& rhs) const
    {
        const KeySet& lhs = *this;
        return (lhs.keys_ & rhs.keys_).any();
    }

    std::string toString() const;
    int toInt() const { return static_cast<int>(keys_.to_ulong()); }

    friend bool operator==(const KeySet& lhs, const KeySet& rhs) { return lhs.keys_ == rhs.keys_; }
    friend bool operator!=(const KeySet& lhs, const KeySet& rhs) { return lhs.keys_ != rhs.keys_; }

private:
    std::bitset<NUM_KEYS> keys_;
};

#endif
