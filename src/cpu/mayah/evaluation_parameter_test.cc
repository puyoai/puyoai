#include "evaluation_parameter.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

TEST(EvaluationParameterTest, toString)
{
    EvaluationParameter param;
    param.setValue(SCORE, 1.0);

    // Since only BOOK parameter is changed, only it's be emitted.
    EXPECT_EQ("SCORE = 1\n", param.toString());
}

TEST(EvaluationParameterMapTest, setter)
{
    EvaluationParameterMap m;
    EXPECT_EQ(0.0, m.defaultParameter().getValue(SCORE));
    EXPECT_EQ(0.0, m.parameter(EvaluationMode::EARLY).getValue(SCORE));

    m.mutableDefaultParameter()->setValue(SCORE, 1.0);
    EXPECT_EQ(1.0, m.defaultParameter().getValue(SCORE));
    EXPECT_EQ(1.0, m.parameter(EvaluationMode::EARLY).getValue(SCORE));
}
