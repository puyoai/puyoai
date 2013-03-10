#include "evaluation_feature.h"

#include <gtest/gtest.h>

using namespace std;

TEST(EvaluationParamsTest, SaveAndLoad)
{
    EvaluationParams original(nullptr);
    original.set(DENSITY_0, 100);
    original.set(MAX_CHAINS, 0, 10);

    original.save("evaluation_feature_test.txt");

    EvaluationParams loaded("evaluation_feature_test.txt");

    // TODO(mayah): We should use EXPECT_EQ here.
    // TODO(mayah): Might fail due to float error.
    EXPECT_TRUE(original == loaded)
        << original.toString() << ' ' << loaded.toString();
}




