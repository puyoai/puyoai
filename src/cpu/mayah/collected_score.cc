#include "collected_score.h"

#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

#include "base/strings.h"

using namespace std;

string CollectedFeatureMoveScore::toString() const
{
    stringstream ss;
    for (const auto& entry : collectedFeatures) {
        ss << toFeature(entry.first).name() << "=" << to_string(entry.second) << endl;
    }
    for (const auto& entry : collectedSparseFeatures) {
        ss << toFeature(entry.first).name() << "=";
        for (int v : entry.second)
            ss << v << ' ';
        ss << endl;
    }

    return ss.str();
}

string CollectedFeatureRensaScore::toString() const
{
    stringstream ss;
    for (const auto& entry : collectedFeatures) {
        ss << toFeature(entry.first).name() << "=" << to_string(entry.second) << endl;
    }

    for (const auto& entry : collectedSparseFeatures) {
        ss << toFeature(entry.first).name() << "=";
        for (int v : entry.second)
            ss << v << ' ';
        ss << endl;
    }

    return ss.str();
}

string CollectedFeatureCoefScore::toString() const
{
    stringstream ss;
    ss << "score = " << score() << endl;
    ss << collectedFeatureScore_.moveScore.toString() << endl;
    ss << collectedFeatureScore_.mainRensaScore.toString() << endl;
    ss << collectedFeatureScore_.sideRensaScore.toString() << endl;

    return ss.str();
}

template<typename FeatureSet, typename CollectedScore, typename ParameterSet>
static string scoreComparisonStringSub(const CollectedScore& lhs,
                                       const CollectedCoef& lhsCoef,
                                       const CollectedScore& rhs,
                                       const CollectedCoef& rhsCoef,
                                       const ParameterSet& paramSet)
{
    typedef typename FeatureSet::FeatureKey FeatureKey;
    typedef typename FeatureSet::SparseFeatureKey SparseFeatureKey;

    set<FeatureKey> featureKeys;
    for (const auto& entry : lhs.collectedFeatures) {
        featureKeys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedFeatures) {
        featureKeys.insert(entry.first);
    }

    set<SparseFeatureKey> sparseFeatureKeys;
    for (const auto& entry : lhs.collectedSparseFeatures) {
        sparseFeatureKeys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedSparseFeatures) {
        sparseFeatureKeys.insert(entry.first);
    }

    stringstream ss;

    for (const auto& key : featureKeys) {
        ss << setw(32) << toFeature(key).name() << " = ";
        ss << setw(12) << fixed << setprecision(3) << lhs.feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << lhs.scoreFor(key, lhsCoef, paramSet) << ") : ";
        ss << setw(12) << fixed << setprecision(3) << rhs.feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << rhs.scoreFor(key, rhsCoef, paramSet) << ")";
        ss << endl;
    }
    for (const auto& key : sparseFeatureKeys) {
        ss << setw(32) << toFeature(key).name() << " = ";
        {
            stringstream st;
            for (int v : lhs.feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << lhs.scoreFor(key, lhsCoef, paramSet) << ")";
        }
        ss << " : ";
        {
            stringstream st;
            for (int v : rhs.feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << rhs.scoreFor(key, rhsCoef, paramSet) << ")";
        }
        ss << endl;
    }

    return ss.str();
}

// static
string CollectedFeatureCoefScore::scoreComparisionString(const CollectedFeatureCoefScore& lhs,
                                                         const CollectedFeatureCoefScore& rhs,
                                                         const EvaluationParameterMap& paramMap)
{
    stringstream ss;
    ss << setw(32) << "WHOLE SCORE" << " = "
       << setw(15) << fixed << setprecision(6) << lhs.score() << "          : "
       << setw(15) << fixed << setprecision(6) << rhs.score() << endl;
    ss << "--------------------------------------------------------------------------------------" << endl;
    ss << scoreComparisonStringSub<EvaluationMoveFeatureSet>(lhs.moveScore(), lhs.coef(),
                                                             rhs.moveScore(), rhs.coef(),
                                                             paramMap.moveParamSet());
    ss << "--------------------------------------------------------------------------------------" << endl;
    ss << scoreComparisonStringSub<EvaluationRensaFeatureSet>(lhs.mainRensaScore(), lhs.coef(),
                                                              rhs.mainRensaScore(), rhs.coef(),
                                                              paramMap.mainRensaParamSet());
    ss << "--------------------------------------------------------------------------------------" << endl;
    ss << scoreComparisonStringSub<EvaluationRensaFeatureSet>(lhs.sideRensaScore(), lhs.coef(),
                                                              rhs.sideRensaScore(), rhs.coef(),
                                                              paramMap.sideRensaParamSet());
    return ss.str();
}
