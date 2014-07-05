#include "core/algorithm/rensa_detector.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace std;

struct ContainsResult {
    ContainsResult(const CoreField& f, const CoreField& g) : f(f), g(g) {}

    bool operator()(const PossibleRensaInfo& info) const {
        CoreField s(f);

        for (const auto& p : info.keyPuyos.list()) {
            s.dropPuyoOn(get<0>(p), get<1>(p));
        }
        for (const auto& p : info.firePuyos.list()) {
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

TEST(RensaDetectorTest, iteratePossibleRensas)
{
    CoreField f("450000"
                "445000"
                "556000");

    bool found = false;
    auto callback = [&found](const CoreField& coreField, const RensaResult& rensaResult,
                             const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        found |= (rensaResult.chains == 3);
    };

    RensaDetector::iteratePossibleRensas(f, 3, callback);
    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, iteratePossibleRensasWithTracking)
{
    CoreField f("450000"
                "445000"
                "556000");

    bool found = false;
    auto callback = [&found](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                             const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        UNUSED_VARIABLE(fieldAfterRensa);
        UNUSED_VARIABLE(keyPuyos);
        UNUSED_VARIABLE(firePuyos);
        found |= (rensaResult.chains == 3);
    };

    RensaDetector::iteratePossibleRensas(f, 3, callback);
    EXPECT_TRUE(found);
}

TEST(RensaDetectorTest, FindPossibleRensasFloat) {
  CoreField f("y     "
              "b     "
              "r     "
              "b     "
              "b     "
              "b     "
              "y     "
              "y     "
              "y     ");

  CoreField g("y     "
              "b     "
              "rr    "
              "br    "
              "br    "
              "bO    "
              "yO    "
              "yO    "
              "yO    ");

  auto expected = g.simulate();
  vector<PossibleRensaInfo> results = RensaDetector::findPossibleRensas(f, 0, RensaDetector::Mode::FLOAT);
  EXPECT_TRUE(std::count_if(results.begin(), results.end(), [&](PossibleRensaInfo result) {
      return result.rensaResult == expected;}));
}
