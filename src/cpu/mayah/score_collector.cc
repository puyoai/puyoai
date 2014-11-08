#include "score_collector.h"

#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

using namespace std;

string CollectedFeature::toString() const
{
    stringstream ss;
    ss << "score = " << score_ << endl;
    for (const auto& entry : collectedFeatures_) {
        ss << EvaluationFeature::toFeature(entry.first).str() << "=" << to_string(entry.second) << endl;
    }
    for (const auto& entry : collectedSparseFeatures_) {
        ss << EvaluationSparseFeature::toFeature(entry.first).str() << "=";
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
        ss << setw(30) << EvaluationFeature::toFeature(key).str() << " = "
           << setw(15) << to_string(feature(key)) << " : "
           << setw(15) << to_string(cf.feature(key)) << endl;
    }
    for (EvaluationSparseFeatureKey key : sparseKeys) {
        ss << setw(30) << EvaluationSparseFeature::toFeature(key).str() << " = ";
        {
            stringstream st;
            for (int v : feature(key)) {
                st << " " << v;
            }
            ss << setw(15) << st.str();
        }
        ss << " : ";
        {
            stringstream st;
            for (int v : cf.feature(key)) {
                st << " " << v;
            }
            ss << setw(15) << st.str();
        }
        ss << endl;
    }

    return ss.str();
}
