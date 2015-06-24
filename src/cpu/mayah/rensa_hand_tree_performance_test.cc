#include "rensa_hand_tree.h"

#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

class RensaHandTreePerformanceTest : public testing::Test {
public:
    RensaHandTreePerformanceTest()
    {
        PuyoPossibility::initialize();
    }
};

TEST(RensaHandTreePerformanceTest, pattern1_depth1)
{
    CoreField cf(
        "    RB"
        " B GGG"
        "GG YBR"
        "YG YGR"
        "GBYBGR"
        "BBYYBG"
        "GYBGRG"
        "GGYGGR"
        "YYBBBR");
    KumipuyoSeq seq("RBRGRYYG");

    for (int i = 0; i < 1000; ++i) {
        std::vector<RensaHandTree> tree = RensaHandTree::makeTree(1, cf, PuyoSet(), 0, seq);
        UNUSED_VARIABLE(tree);
    }
}

TEST(RensaHandTreePerformanceTest, pattern1_depth2)
{
    CoreField cf(
        "    RB"
        " B GGG"
        "GG YBR"
        "YG YGR"
        "GBYBGR"
        "BBYYBG"
        "GYBGRG"
        "GGYGGR"
        "YYBBBR");
    KumipuyoSeq seq("RBRGRYYG");

    for (int i = 0; i < 1000; ++i) {
        std::vector<RensaHandTree> tree = RensaHandTree::makeTree(2, cf, PuyoSet(), 0, seq);
        UNUSED_VARIABLE(tree);
    }
}

TEST(RensaHandTreePerformanceTest, pattern1_depth3)
{
    CoreField cf(
        "    RB"
        " B GGG"
        "GG YBR"
        "YG YGR"
        "GBYBGR"
        "BBYYBG"
        "GYBGRG"
        "GGYGGR"
        "YYBBBR");
    KumipuyoSeq seq("RBRGRYYG");

    for (int i = 0; i < 1000; ++i) {
        std::vector<RensaHandTree> tree = RensaHandTree::makeTree(3, cf, PuyoSet(), 0, seq);
        UNUSED_VARIABLE(tree);
    }
}
