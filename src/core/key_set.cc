#include "core/key_set.h"

#include <sstream>

#include "base/strings.h"

using namespace std;

KeySet::KeySet(Key key)
{
    setKey(key);
}

KeySet::KeySet(Key key1, Key key2)
{
    setKey(key1);
    setKey(key2);
}

void KeySet::setKey(Key key, bool flag)
{
    keys_.set(ordinal(key), flag);
}

bool KeySet::hasKey(Key key) const
{
    return keys_.test(ordinal(key));
}

string KeySet::toString() const
{
    stringstream ss;
    if (hasKey(Key::LEFT))
        ss << "<";
    if (hasKey(Key::RIGHT))
        ss << ">";
    if (hasKey(Key::DOWN))
        ss << "v";
    if (hasKey(Key::LEFT_TURN))
        ss << "B";
    if (hasKey(Key::RIGHT_TURN))
        ss << "A";

    return ss.str();
}

KeySetSeq::KeySetSeq(const string& str)
{
    for (const auto& s : strings::split(str, ',')) {
        KeySet ks;
        for (const auto& c : s)
            ks.setKey(toKey(c));
        seq_.push_back(ks);
    }
}

std::string KeySetSeq::toString() const
{
    // caution: this string is used by test cases.
    stringstream ss;

    for (size_t i = 0; i < seq_.size(); ++i) {
        if (i != 0)
            ss << ',';
        ss << seq_[i].toString();
    }

    return ss.str();
}
