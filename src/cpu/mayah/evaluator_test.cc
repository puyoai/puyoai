#include "evaluator.h"

#include <gtest/gtest.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/decision.h"
#include "core/field/core_field.h"
#include "gazer.h"

using namespace std;

class EvaluatorTest : public testing::Test {
protected:
    CollectedFeature eval(const CoreField& f)
    {
        TsumoPossibility::initialize();

        FeatureParameter parameter;
        Evaluator evaluator(parameter);
        Gazer gazer;

        gazer.initializeWith(1);

        vector<Decision> decisions { Decision(3, 0) };
        RensaResult rensaResult;
        int initiatingFrames = 10;

        RefPlan plan(f, decisions, rensaResult, initiatingFrames);

        return evaluator.evalWithCollectingFeature(plan, f, 1, gazer);
    }
};

TEST_F(EvaluatorTest, ConnectionHorizontal1)
{
    CoreField f(
        "B B B "
        "GG YY "
        "RRRGGG");
    CollectedFeature cf = eval(f);

    map<int, int> m;
    for (auto v : cf.collectedSparseFeatures[CONNECTION_HORIZONTAL])
        m[v]++;

    EXPECT_EQ(2, m[3]);
    EXPECT_EQ(2, m[2]);
    EXPECT_EQ(3, m[1]);
    EXPECT_EQ(0, m[0]);
}
