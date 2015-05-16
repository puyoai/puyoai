#include "core/key_set_seq.h"

#include <sstream>

#include "base/strings.h"

using namespace std;

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
