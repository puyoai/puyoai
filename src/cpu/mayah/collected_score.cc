#include "collected_score.h"

#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

using namespace std;

string CollectedFeatureCoefScore::toString() const
{
    stringstream ss;
    ss << "score = " << score() << endl;
    for (const auto& entry : collectedFeatureScore_.collectedFeatures) {
        ss << EvaluationFeature::toFeature(entry.first).str() << "=" << to_string(entry.second) << endl;
    }
    for (const auto& entry : collectedFeatureScore_.collectedSparseFeatures) {
        ss << EvaluationSparseFeature::toFeature(entry.first).str() << "=";
        for (int v : entry.second)
            ss << v << ' ';
        ss << endl;
    }

    return ss.str();
}

string CollectedFeatureCoefScore::toStringComparingWith(const CollectedFeatureCoefScore& cf, const EvaluationParameterMap& paramMap) const
{
    const CollectedFeatureCoefScore& lhs = *this;
    const CollectedFeatureCoefScore& rhs = cf;

    set<EvaluationFeatureKey> keys;
    for (const auto& entry : lhs.collectedFeatureScore_.collectedFeatures) {
        keys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedFeatureScore_.collectedFeatures) {
        keys.insert(entry.first);
    }

    set<EvaluationSparseFeatureKey> sparseKeys;
    for (const auto& entry : lhs.collectedFeatureScore_.collectedSparseFeatures) {
        sparseKeys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedFeatureScore_.collectedSparseFeatures) {
        sparseKeys.insert(entry.first);
    }

    stringstream ss;
    ss << setw(30) << "WHOLE SCORE" << " = "
       << setw(15) << fixed << setprecision(6) << lhs.score() << "          : "
       << setw(15) << fixed << setprecision(6) << rhs.score() << endl;
    ss << "----------------------------------------------------------------------" << endl;
    for (EvaluationFeatureKey key : keys) {
        ss << setw(30) << EvaluationFeature::toFeature(key).str() << " = ";
        ss << setw(12) << fixed << setprecision(3) << lhs.feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << lhs.scoreFor(key, paramMap) << ") : ";
        ss << setw(12) << fixed << setprecision(3) << rhs.feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << rhs.scoreFor(key, paramMap) << ")";
        ss << endl;
    }
    for (EvaluationSparseFeatureKey key : sparseKeys) {
        ss << setw(30) << EvaluationSparseFeature::toFeature(key).str() << " = ";
        {
            stringstream st;
            for (int v : lhs.feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << lhs.scoreFor(key, paramMap) << ")";
        }
        ss << " : ";
        {
            stringstream st;
            for (int v : rhs.feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << rhs.scoreFor(key, paramMap) << ")";
        }
        ss << endl;
    }

    return ss.str();
}
