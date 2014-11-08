#include "evaluation_parameter.h"

#include <fstream>
#include <map>
#include <sstream>
#include <vector>

#include <gflags/gflags.h>

#include "core/field/core_field.h"

using namespace std;

EvaluationParameter::EvaluationParameter() :
    coef_(EvaluationFeature::all().size()),
    sparseCoef_(EvaluationSparseFeature::all().size())
{
    for (const EvaluationSparseFeature& feature : EvaluationSparseFeature::all()) {
        sparseCoef_[feature.key()].resize(feature.size());
    }
}

EvaluationParameter::EvaluationParameter(const string& filename) :
    coef_(EvaluationFeature::all().size()),
    sparseCoef_(EvaluationSparseFeature::all().size())
{
    CHECK(load(filename));
}

bool EvaluationParameter::save(const string& filename)
{
    try {
        ofstream ofs(filename, ios::out | ios::trunc);

        for (const auto& ef : EvaluationFeature::all()) {
            ofs << ef.str() << " = " << getValue(ef.key()) << endl;
        }
        for (const auto& ef : EvaluationSparseFeature::all()) {
            ofs << ef.str() << " =";
            for (size_t i = 0; i < ef.size(); ++i) {
                ofs << ' ' << getValue(ef.key(), i);
            }
            ofs << endl;
        }

        return true;
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationParameter::save failed: " << e.what();
        return false;
    }
}

bool EvaluationParameter::load(const string& filename)
{
    for (const EvaluationSparseFeature& feature : EvaluationSparseFeature::all()) {
        sparseCoef_[feature.key()].resize(feature.size());
    }

    map<string, vector<double>> keyValues;
    try {
        ifstream ifs(filename, ios::in);

        string str;
        while (getline(ifs, str)) {
            while (!str.empty() && str.back() == '\\') {
                str.back() = ' ';
                string line;
                if (getline(ifs, line)) {
                    str += line;
                }
            }

            istringstream ss(str);
            string key;
            if (!(ss >> key))
                continue;

            char c;
            if (!((ss >> c) && c == '=')) {
                LOG(WARNING) << "Invalid EvaluationParameterFormat: "
                             << key << " : "
                             << str;
                return false;
            }

            vector<double> values;
            double v;
            while (ss >> v)
                values.push_back(v);

            keyValues.insert(make_pair(key, values));
        }
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationParameter::load failed: " << e.what();
        return false;
    }

    for (const EvaluationFeature& ef : EvaluationFeature::all()) {
        if (keyValues.count(ef.str())) {
            coef_[ef.key()] = keyValues[ef.str()].front();
            keyValues.erase(ef.str());
        }
    }

    for (const EvaluationSparseFeature& ef : EvaluationSparseFeature::all()) {
        if (keyValues.count(ef.str())) {
            if (ef.size() != keyValues[ef.str()].size()) {
                LOG(ERROR) << "Invalid EvaluationParameterFormat: "
                           << ef.str() << " : Length mismatch.";
                return false;
            }

            sparseCoef_[ef.key()] = keyValues[ef.str()];
            keyValues.erase(ef.str());
        }
    }

    if (!keyValues.empty()) {
        stringstream ss;
        bool first = true;
        for (const auto& entry : keyValues) {
            if (first) {
                first = false;
            } else {
                ss << ",";
            }
            ss << entry.first;
        }
        CHECK(false) << "Unknown feature key is specified: " << ss.str();
    }

    return true;
}

string EvaluationParameter::toString() const
{
    ostringstream ss;

    for (const EvaluationFeature& ef : EvaluationFeature::all()) {
        ss << ef.str() << " = " << coef_[ef.key()] << endl;
    }

    for (const EvaluationSparseFeature& ef : EvaluationSparseFeature::all()) {
        ss << ef.str() << " =";
        for (size_t i = 0; i < ef.size(); ++i) {
            ss << ' ' << sparseCoef_[ef.key()][i];
        }
        ss << endl;
    }

    return ss.str();
}

bool operator==(const EvaluationParameter& lhs, const EvaluationParameter& rhs)
{
    return lhs.coef_ == rhs.coef_ && lhs.sparseCoef_ == rhs.sparseCoef_;
}
