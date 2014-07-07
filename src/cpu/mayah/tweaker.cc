#include <random>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "evaluation_feature_key.h"
#include "feature_parameter.h"

DECLARE_string(feature);

using namespace std;

void tweakParameter(FeatureParameter* param)
{
    random_device rd;
    mt19937 mt(rd());

#define DEFINE_PARAM(key)                                       \
    switch (mt() % 20) {                                        \
    case 0:                                                     \
        param->setValue(key, param->getValue(key) + mt() % 20); \
        break;                                                  \
    case 1:                                                     \
        param->setValue(key, param->getValue(key) - mt() % 20); \
        break;                                                  \
    default:                                                    \
        break;                                                  \
    }
#define DEFINE_SPARSE_PARAM(key, maxVlaue)                   \
    switch (mt() % 20) {                                     \
    case 0: {                                                \
        vector<double> vs = param->getValues(key);           \
        double diff = mt() % 20;                             \
        for (size_t i = 0; i < vs.size(); ++i) {             \
            vs[i] += diff;                                   \
        }                                                    \
        param->setValues(key, vs);                           \
        break;                                               \
    }                                                        \
    case 1: {                                                \
        vector<double> vs = param->getValues(key);           \
        double diff = mt() % 20;                             \
        for (size_t i = 0; i < vs.size(); ++i) {             \
            vs[i] -= diff;                                   \
        }                                                    \
        param->setValues(key, vs);                           \
        break;                                               \
    }                                                        \
    default:                                                 \
        break;                                               \
    }
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    FeatureParameter param;
    CHECK(param.load(FLAGS_feature));
    tweakParameter(&param);
    CHECK(param.save(FLAGS_feature));

    return 0;
}
