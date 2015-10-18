#include "pattern_tree.h"

using namespace std;

PatternTree* PatternTree::put(const PatternBit& patternBit)
{
    for (auto& entry : children_) {
        if (entry.first == patternBit)
            return entry.second.get();
    }

    children_.emplace_back(patternBit, unique_ptr<PatternTree>(new PatternTree()));
    return children_.back().second.get();
}

bool PatternTree::setLeaf(const std::string& name,
                          const FieldBits& ironBits,
                          const FieldBits& mustBits,
                          int ignitionColumn,
                          int numVariables,
                          double score)
{
    CHECK(0 <= ignitionColumn && ignitionColumn <= 6);
    if (patternBookField_.get())
        return false;

    patternBookField_.reset(new PatternBookField(name, ironBits, mustBits, ignitionColumn, numVariables, score));
    return true;
}
