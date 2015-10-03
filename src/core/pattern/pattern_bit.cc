#include "core/pattern/pattern_bit.h"

using namespace std;

int PatternBitSet::put(const PatternBit& pb)
{
    auto it = registeredBits_.find(pb);
    if (it != registeredBits_.end())
        return it->second;

    int id = static_cast<int>(patternBits_.size());
    registeredBits_.emplace(pb, id);
    patternBits_.push_back(pb);
    return id;
}
