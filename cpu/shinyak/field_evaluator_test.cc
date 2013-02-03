#include "field_evaluator.h"

#include <iostream>
#include <vector>
#include <gtest/gtest.h>

#include "field.h"

using namespace std;

TEST(FieldEvaluatorTest, CalculateEmptyFieldAvailabilityTest)
{
    Field field1(
        "000006"
        "547444"
        "554777"
        "446664");
    Field field2(
        "040000"
        "050000"
        "050000"
        "056740"
        "045674"
        "045674"
        "045674");

    EXPECT_GT(FieldEvaluator::calculateEmptyFieldAvailability(field1),
              FieldEvaluator::calculateEmptyFieldAvailability(field2));
}
