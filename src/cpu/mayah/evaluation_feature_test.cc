#include "evaluation_feature.h"

#include <gtest/gtest.h>

using namespace std;

TEST(EvaluationFeatureTest, SaveAndLoad)
{
    EvaluationFeature original(nullptr);
    original.setValue(DENSITY_0, 100);
    original.setValue(MAX_CHAINS, 0, 10);

    original.save("evaluation_feature_test.txt");

    EvaluationFeature loaded("evaluation_feature_test.txt");

    // TODO(mayah): We should use EXPECT_EQ here.
    // TODO(mayah): Might fail due to float error.
    EXPECT_EQ(original, loaded)
        << original.toString() << ' ' << loaded.toString();
}




