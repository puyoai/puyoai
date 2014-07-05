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

static void dropKeyAndFirePuyos(CoreField* f, const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos)
{
    for (const auto& p : keyPuyos.list()) {
        f->dropPuyoOn(get<0>(p), get<1>(p));
    }
    for (const auto& p : firePuyos.list()) {
        f->dropPuyoOn(get<0>(p), get<1>(p));
    }
}

TEST(RensaDetectorTest, FindPossibleRensaTest)
{
    CoreField f(" BRR  "
                " RBR  "
                "BBYYY ");

    CoreField expected1(" BRR  "
                        " RBR  "
                        "BBYYYY");

    CoreField expected2(" BRR  "
                        " RBRY "
                        "BBYYY ");

    bool found1 = false;
    bool found2 = false;
    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        CoreField actual(f);
        dropKeyAndFirePuyos(&actual, keyPuyos, firePuyos);
        if (actual == expected1)
            found1 = true;
        if (actual == expected2)
            found2 = true;
    };

    RensaDetector::iteratePossibleRensas(f, 0, callback);
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
}

TEST(RensaDetectorTest, FindPossibleRensasWithKeyPuyo)
{
    CoreField f("RB    "
                "RRB   "
                "BBY   ");

    CoreField expected[] {
        CoreField("R     "
                  "RBY   "
                  "RRBY  "
                  "BBYY  "),
        CoreField("R     "
                  "RBY   "
                  "RRB   "
                  "BBYYY "),
        CoreField("RY    "
                  "RBY   "
                  "RRB   "
                  "BBYY  "),
        CoreField("RY    "
                  "RB    "
                  "RRB   "
                  "BBYYY ")
    };

    bool found[4] {};
    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        CoreField actual(f);
        dropKeyAndFirePuyos(&actual, keyPuyos, firePuyos);
        for (int i = 0; i < 4; ++i) {
            if (actual == expected[i]) {
                found[i] = true;
                break;
            }
        }
    };

    RensaDetector::iteratePossibleRensas(f, 3, callback);

    EXPECT_TRUE(found[0]);
    EXPECT_TRUE(found[1]);
    EXPECT_TRUE(found[2]);
    EXPECT_TRUE(found[3]);
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
