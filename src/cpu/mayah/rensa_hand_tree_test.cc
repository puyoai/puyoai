#include "rensa_hand_tree.h"

#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

class RensaHandTreeTest : public testing::Test {
public:
    RensaHandTreeTest()
    {
        PuyoPossibility::initialize();
    }
};

TEST(RensaHandTreeTest, eval_empty)
{
    const std::vector<RensaHandTree> empty;
    EXPECT_EQ(0, RensaHandTree::eval(empty, 0, 0, 0, empty, 0, 0, 0));
}

TEST(RensaHandTreeTest, eval_5rensa)
{
    const int NUM_FRAMES_OF_ONE_RENSA = FRAMES_VANISH_ANIMATION + FRAMES_GROUNDING + FRAMES_TO_DROP_FAST[1];

    RensaCoefResult coefResult;
    coefResult.setCoef(1, 4, 0, 0);
    coefResult.setCoef(2, 4, 0, 0);
    coefResult.setCoef(3, 4, 0, 0);
    coefResult.setCoef(4, 4, 0, 0);
    coefResult.setCoef(5, 4, 0, 0);

    RensaHand fiveRensa(IgnitionRensaResult(RensaResult(5, 40 * (1 + 8 + 16 + 32 + 64), 5 * NUM_FRAMES_OF_ONE_RENSA, false),
                                            0),
                        coefResult);

    const std::vector<RensaHandTree> myTree {
        RensaHandTree(fiveRensa, std::vector<RensaHandTree>()),
    };

    const std::vector<RensaHandTree> enemyTree;

    // 5rensa = 69, 3rensa = 14, 2rensa = 5
    EXPECT_EQ(69 - 5, RensaHandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0));
    EXPECT_EQ(5 - 69, RensaHandTree::eval(enemyTree, 0, 0, 0, myTree, 0, 0, 0));
}

TEST(RensaHandTreeTest, eval_actual1)
{
    PuyoPossibility::initialize();

    const CoreField cf1(
        ".....R"
        "....GR"
        "G..YYY"
        "YYYGGR"
        "GRBGYR"
        "GGRBBB"
        "RRBYYY");

    const CoreField cf2(
        ".RBYG."
        "RBYGR."
        "RBYGR."
        "RBYGR.");

    std::vector<RensaHandTree> myTree = RensaHandTree::makeTree(2, cf1, PuyoSet(), 0, KumipuyoSeq("YYYY"));
    std::vector<RensaHandTree> enemyTree = RensaHandTree::makeTree(2, cf2, PuyoSet(), 0, KumipuyoSeq("YYGG"));

    EXPECT_LT(0, RensaHandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0));

#if 0
    for (const auto& t : myTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    for (const auto& t : enemyTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;

    cout << RensaHandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0) << endl;
    cout << RensaHandTree::eval(enemyTree, 0, 0, 0, myTree, 0, 0, 0) << endl;
#endif
}

TEST(RensaHandTreeTest, eval_actual2)
{
    PuyoPossibility::initialize();

    const CoreField cf1(
        "......"
        "......"
        "B...GY"
        "B...BB"
        "RR.GGB"
        "BR.GYY"
        "BYGRBY"
        "RBYGRB"
        "RBYGRB"
        "RBYGRB");

    const CoreField cf2(
        "B....."
        "R....."
        "B....."
        "B....."
        "RR...."
        "BROOOO"
        "BYGRBO"
        "RBYGRB"
        "RBYGRB"
        "RBYGRB");

    std::vector<RensaHandTree> myTree = RensaHandTree::makeTree(2, cf1, PuyoSet(), 0, KumipuyoSeq());
    std::vector<RensaHandTree> enemyTree = RensaHandTree::makeTree(2, cf2, PuyoSet(), 0, KumipuyoSeq());

    EXPECT_LT(0, RensaHandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0));

#if 0
    for (const auto& t : myTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    for (const auto& t : enemyTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    cout << RensaHandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0) << endl;
#endif
}
