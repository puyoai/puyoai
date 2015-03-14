#include "feature_collector.h"

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

string CollectedFeature::toStringComparingWith(const CollectedFeature& cf, const EvaluationParameter& param) const
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
    ss << setw(30) << "WHOLE SCORE" << " = "
       << setw(15) << fixed << setprecision(6) << score_ << "          : "
       << setw(15) << fixed << setprecision(6) << cf.score_ << endl;
    ss << "----------------------------------------------------------------------" << endl;
    for (EvaluationFeatureKey key : keys) {
        ss << setw(30) << EvaluationFeature::toFeature(key).str() << " = ";
        ss << setw(12) << fixed << setprecision(3) << feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << param.score(key, feature(key)) << ") : ";
        ss << setw(12) << fixed << setprecision(3) << cf.feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << param.score(key, cf.feature(key)) << ")";
        ss << endl;
    }
    for (EvaluationSparseFeatureKey key : sparseKeys) {
        ss << setw(30) << EvaluationSparseFeature::toFeature(key).str() << " = ";
        {
            double d = 0;
            stringstream st;
            for (int v : feature(key)) {
                d += param.score(key, v, 1);
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << d << ")";
        }
        ss << " : ";
        {
            double d = 0;
            stringstream st;
            for (int v : cf.feature(key)) {
                st << " " << v;
                d += param.score(key, v, 1);
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << d << ")";
        }
        ss << endl;
    }

    return ss.str();
}
