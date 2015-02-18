#include "evaluation_parameter.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

TEST(EvaluationParameterTest, toString)
{
    EvaluationParameter param;
    param.setValue(BOOK, 1.0);

    // Since only BOOK parameter is changed, only it's be emitted.
    EXPECT_EQ("BOOK = 1\n", param.toString());
}

TEST(EvaluationParameterMapTest, toString)
{
    EvaluationParameter param;
    param.setValue(BOOK, 1.0);

    // Since only BOOK parameter is changed, only it's be emitted.
    EXPECT_EQ("BOOK = 1\n", param.toString());
}
