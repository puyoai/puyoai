#include "score_collector.h"

#include <iostream>
#include <set>
#include <sstream>

using namespace std;

string CollectedFeature::toString() const
{
    stringstream ss;
    ss << "score = " << score_ << endl;
    for (const auto& entry : collectedFeatures_) {
        ss << ::toString(entry.first) << "=" << to_string(entry.second) << endl;
    }
    for (const auto& entry : collectedSparseFeatures_) {
        ss << ::toString(entry.first) << "=";
        for (int v : entry.second)
            ss << v << ' ';
        ss << endl;
    }

    return ss.str();
}

string CollectedFeature::toStringComparingWith(const CollectedFeature& cf) const
{
    set<EvaluationFeatureKey> keys;
    for (const auto& entry : collectedFeatures_) {
        keys.insert(entry.first);
    }
    for (const auto& entry : cf.collectedFeatures_) {
        keys.insert(entry.first);
    }

    set<EvaluationSparseFeatureKey> sparseKeys;
    for (const auto& entry : collectedSparseFeatures_) {
        sparseKeys.insert(entry.first);
    }
    for (const auto& entry : cf.collectedSparseFeatures_) {
        sparseKeys.insert(entry.first);
    }

    stringstream ss;
    ss << "score = " << score_ << " : " << cf.score_ << endl;
    for (EvaluationFeatureKey key : keys) {
        ss << ::toString(key) << " = "
           << to_string(feature(key)) << " : "
           << to_string(cf.feature(key)) << endl;
    }
    for (EvaluationSparseFeatureKey key : sparseKeys) {
        ss << ::toString(key) << " =";
        for (int v : feature(key)) {
            ss << " " << v;
        }
        ss << " :";
        for (int v : cf.feature(key)) {
            ss << " " << v;
        }
        ss << endl;
    }

    return ss.str();
}
