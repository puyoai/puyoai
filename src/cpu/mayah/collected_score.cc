#include "collected_score.h"

#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

using namespace std;

string CollectedFeatureMoveScore::toString() const
{
    stringstream ss;
    for (const auto& entry : collectedFeatures) {
        ss << EvaluationFeature::toFeature(entry.first).str() << "=" << to_string(entry.second) << endl;
    }
    for (const auto& entry : collectedSparseFeatures) {
        ss << EvaluationSparseFeature::toFeature(entry.first).str() << "=";
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
        ss << EvaluationFeature::toFeature(entry.first).str() << "=" << to_string(entry.second) << endl;
    }

    for (const auto& entry : collectedSparseFeatures) {
        ss << EvaluationSparseFeature::toFeature(entry.first).str() << "=";
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
    ss << collectedFeatureScore_.rensaScore.toString() << endl;

    return ss.str();
}

// static
string CollectedFeatureCoefScore::scoreComparisionString(const CollectedFeatureCoefScore& lhs,
                                                         const CollectedFeatureCoefScore& rhs,
                                                         const EvaluationParameterMap& paramMap)
{
    set<EvaluationFeatureKey> moveScoreKeys;
    for (const auto& entry : lhs.collectedFeatureScore_.moveScore.collectedFeatures) {
        moveScoreKeys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedFeatureScore_.moveScore.collectedFeatures) {
        moveScoreKeys.insert(entry.first);
    }

    set<EvaluationSparseFeatureKey> moveSparseScoreKeys;
    for (const auto& entry : lhs.collectedFeatureScore_.moveScore.collectedSparseFeatures) {
        moveSparseScoreKeys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedFeatureScore_.moveScore.collectedSparseFeatures) {
        moveSparseScoreKeys.insert(entry.first);
    }

    set<EvaluationFeatureKey> rensaScoreKeys;
    for (const auto& entry : lhs.collectedFeatureScore_.rensaScore.collectedFeatures) {
        rensaScoreKeys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedFeatureScore_.rensaScore.collectedFeatures) {
        rensaScoreKeys.insert(entry.first);
    }

    set<EvaluationSparseFeatureKey> rensaSparseScoreKeys;
    for (const auto& entry : lhs.collectedFeatureScore_.rensaScore.collectedSparseFeatures) {
        rensaSparseScoreKeys.insert(entry.first);
    }
    for (const auto& entry : rhs.collectedFeatureScore_.rensaScore.collectedSparseFeatures) {
        rensaSparseScoreKeys.insert(entry.first);
    }

    stringstream ss;
    ss << setw(32) << "WHOLE SCORE" << " = "
       << setw(15) << fixed << setprecision(6) << lhs.score() << "          : "
       << setw(15) << fixed << setprecision(6) << rhs.score() << endl;
    ss << "--------------------------------------------------------------------------------------" << endl;
    for (EvaluationFeatureKey key : moveScoreKeys) {
        ss << setw(32) << EvaluationFeature::toFeature(key).str() << " = ";
        ss << setw(12) << fixed << setprecision(3) << lhs.moveScore().feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << lhs.moveScore().scoreFor(key, lhs.coef(), paramMap) << ") : ";
        ss << setw(12) << fixed << setprecision(3) << rhs.moveScore().feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << rhs.moveScore().scoreFor(key, rhs.coef(), paramMap) << ")";
        ss << endl;
    }
    for (EvaluationSparseFeatureKey key : moveSparseScoreKeys) {
        ss << setw(32) << EvaluationSparseFeature::toFeature(key).str() << " = ";
        {
            stringstream st;
            for (int v : lhs.moveScore().feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << lhs.moveScore().scoreFor(key, lhs.coef(), paramMap) << ")";
        }
        ss << " : ";
        {
            stringstream st;
            for (int v : rhs.moveScore().feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << rhs.moveScore().scoreFor(key, rhs.coef(), paramMap) << ")";
        }
        ss << endl;
    }
    ss << "--------------------------------------------------------------------------------------" << endl;
    for (EvaluationFeatureKey key : rensaScoreKeys) {
        ss << setw(32) << EvaluationFeature::toFeature(key).str() << " = ";
        ss << setw(12) << fixed << setprecision(3) << lhs.rensaScore().feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << lhs.rensaScore().scoreFor(key, lhs.coef(), paramMap) << ") : ";
        ss << setw(12) << fixed << setprecision(3) << rhs.rensaScore().feature(key) << " ("
           << setw(9) << fixed << setprecision(3) << rhs.rensaScore().scoreFor(key, rhs.coef(), paramMap) << ")";
        ss << endl;
    }
    for (EvaluationSparseFeatureKey key : rensaSparseScoreKeys) {
        ss << setw(32) << EvaluationSparseFeature::toFeature(key).str() << " = ";
        {
            stringstream st;
            for (int v : lhs.rensaScore().feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << lhs.rensaScore().scoreFor(key, lhs.coef(),paramMap) << ")";
        }
        ss << " : ";
        {
            stringstream st;
            for (int v : rhs.rensaScore().feature(key)) {
                st << " " << v;
            }
            ss << setw(12) << st.str() << " ("
               << setw(9) << fixed << setprecision(3) << rhs.rensaScore().scoreFor(key, rhs.coef(), paramMap) << ")";
        }
        ss << endl;
    }
    return ss.str();
}
