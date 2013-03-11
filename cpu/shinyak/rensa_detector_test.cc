#include "rensa_detector.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace std;

struct ContainsRensa {
    ContainsRensa(int chains, PuyoSet set) : chains(chains), set(set) {}
    
    bool operator()(const PossibleRensaInfo& info) const {
        return chains == info.rensaInfo.chains && info.necessaryPuyoSet == set;
    }

    bool operator()(const TrackedPossibleRensaInfo& info) const {
        return chains == info.rensaInfo.chains && info.necessaryPuyoSet == set;
    }
    
    int chains;
    PuyoSet set;
};

struct ContainsTrackedRensa {
    ContainsTrackedRensa(int chains, PuyoSet set, int tracked[][6], int height)
        : chains(chains)
        , set(set)
        , tracked(tracked)
        , height(height) {}

    bool operator()(const TrackedPossibleRensaInfo& info) const {
        if (chains != info.rensaInfo.chains)
            return false;
        if (info.necessaryPuyoSet != set)
            return false;

        for (int y = 1; y <= height; ++y) {
            for (int x = 1; x <= Field::WIDTH; ++x) {
                if (info.trackResult.erasedAt(x, y) != tracked[y-1][x-1])
                    return false;
            }
        }
        return true;
    }

    int chains;
    PuyoSet set;
    int (*tracked)[6];
    int height;
};

TEST(RensaDetectorTest, FindPossibleRensaTest1)
{
    Field f("054400"
            "045400"
            "556660");

    vector<PossibleRensaInfo> result;
    RensaDetector::findPossibleRensas(result, f);

    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(1, PuyoSet(1, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(2, PuyoSet(3, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(3, PuyoSet(0, 0, 1, 0))));
}

TEST(RensaDetectorTest, FindPossibleRensas2)
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

TEST(RensaDetectorTest, FindPossibleRensaTestWithTracking)
{
    Field f("054400"
            "045400"
            "556660");

    int tracked[3][6] = {
        { 3, 3, 1, 1, 1, 1, },
        { 0, 2, 3, 2, 0, 0, },
        { 0, 3, 2, 2, 0, 0, },
    };

    vector<TrackedPossibleRensaInfo> result;
    RensaDetector::findPossibleRensas(result, f);

    EXPECT_TRUE(std::count_if(result.begin(), result.end(), 
                              ContainsTrackedRensa(3, PuyoSet(0, 0, 1, 0), tracked, 3)));
}

TEST(RensaDetectorTest, FindPossibleRensasUsingIteration3)
{    
    Field f("450000"
            "445660"
            "556455");

    vector<vector<TrackedPossibleRensaInfo>> result;
    RensaDetector::findPossibleRensasUsingIteration(result, f, 3);

    EXPECT_TRUE(std::count_if(result[3].begin(), result[3].end(), ContainsRensa(4, PuyoSet(1, 2, 1, 0))));
}
