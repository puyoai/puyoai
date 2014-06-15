#include "core/algorithm/rensa_detector.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace std;

struct ContainsResult {
    ContainsResult(const CoreField& f, const CoreField& g) : f(f), g(g) {}

    bool operator()(const PossibleRensaInfo& info) const {
        CoreField s(f);

        for (const auto& p : info.necessaryPuyoSet.list()) {
            s.dropPuyoOn(get<0>(p), get<1>(p));
        }

        if (s != g)
            return false;

        auto result = s.simulate();
        return result == info.rensaResult;
    }

    const CoreField& f;
    const CoreField& g;
};

TEST(RensaDetectorTest, FindPossibleRensaTest)
{
    CoreField f("054400"
                "045400"
                "556660");

    CoreField g("054400"
                "045400"
                "556666");

    vector<PossibleRensaInfo> result = RensaDetector::findPossibleRensas(f);
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsResult(f, g)));
}

TEST(RensaDetectorTest, FindPossibleRensasWithKeyPuyo)
{
    CoreField f("450000"
                "445000"
                "556000");

    CoreField g("400000"
                "456000"
                "445600"
                "556600");

    vector<PossibleRensaInfo> result = RensaDetector::findPossibleRensas(f, 3);
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsResult(f, g)));
}

