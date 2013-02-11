#include "rensa_detector.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace std;

struct ContainsRensa {
    ContainsRensa(int chains, PuyoSet set) : chains(chains), set(set) {}
    
    bool operator()(const PossibleRensaInfo& info) const {
        return chains == info.rensaInfo.chains && info.necessaryPuyoSet == set;
    }
    
    int chains;
    PuyoSet set;
};

TEST(RensaDetectorTest, FindRensaTest)
{
    Field f("054400"
            "045400"
            "556660");

    vector<PossibleRensaInfo> result;
    RensaDetector::findRensas(result, f, PuyoSet());

    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(1, PuyoSet(1, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(2, PuyoSet(3, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(3, PuyoSet(0, 0, 1, 0))));
}

TEST(RensaDetectorTest, FindPossibleRensas)
{
    Field f("450000"
            "445000"
            "556000");

    vector<PossibleRensaInfo> result;
    RensaDetector::findPossibleRensas(result, f, 3);

    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(1, PuyoSet(0, 2, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(2, PuyoSet(1, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(3, PuyoSet(1, 0, 3, 0))));
}

TEST(RensaDetectorTest, FindPossibleRensasUsingIteration1)
{    
    Field f("450000"
            "445660"
            "556455");

    vector<PossibleRensaInfo> result;
    RensaDetector::findPossibleRensasUsingIteration(result, f, 3);

    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(4, PuyoSet(1, 2, 1, 0))));
}
