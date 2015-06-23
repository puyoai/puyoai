#include "hand_tree.h"

#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

class HandTreeTest : public testing::Test {
public:
    HandTreeTest()
    {
        PuyoPossibility::initialize();
    }
};

TEST(HandTreeTest, eval_empty)
{
    const std::vector<EstimatedRensaInfoTree> empty;
    EXPECT_EQ(0, HandTree::eval(empty, 0, 0, 0, empty, 0, 0, 0));
}

TEST(HandTreeTest, eval_5rensa)
{
    const int NUM_FRAMES_OF_ONE_RENSA = FRAMES_PREPARING_NEXT + FRAMES_VANISH_ANIMATION + FRAMES_GROUNDING + FRAMES_TO_DROP_FAST[1];

    RensaCoefResult coefResult;
    coefResult.setCoef(1, 4, 0, 0);
    coefResult.setCoef(2, 4, 0, 0);
    coefResult.setCoef(3, 4, 0, 0);
    coefResult.setCoef(4, 4, 0, 0);
    coefResult.setCoef(5, 4, 0, 0);

    EstimatedRensaInfo fiveRensa(IgnitionRensaResult(RensaResult(5, 40 * (1 + 8 + 16 + 32 + 64), 5 * NUM_FRAMES_OF_ONE_RENSA, false),
                                                     0),
                                 coefResult);

    const std::vector<EstimatedRensaInfoTree> myTree {
        EstimatedRensaInfoTree(fiveRensa, std::vector<EstimatedRensaInfoTree>()),
    };

    const std::vector<EstimatedRensaInfoTree> enemyTree;

    // 5rensa = 69, 3rensa = 14
    EXPECT_EQ(69 - 14, HandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0));
    EXPECT_EQ(14 - 69, HandTree::eval(enemyTree, 0, 0, 0, myTree, 0, 0, 0));
}

TEST(HandTreeTest, eval_actual1)
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

    std::vector<EstimatedRensaInfoTree> myTree = HandTree::makeTree(2, cf1, PuyoSet(), 0, KumipuyoSeq("YYYY"));
    std::vector<EstimatedRensaInfoTree> enemyTree = HandTree::makeTree(2, cf2, PuyoSet(), 0, KumipuyoSeq("YYGG"));

    EXPECT_LT(0, HandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0));

#if 0
    for (const auto& t : myTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    for (const auto& t : enemyTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;

    cout << HandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0) << endl;
    cout << HandTree::eval(enemyTree, 0, 0, 0, myTree, 0, 0, 0) << endl;
#endif
}

TEST(HandTreeTest, eval_actual2)
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

    std::vector<EstimatedRensaInfoTree> myTree = HandTree::makeTree(2, cf1, PuyoSet(), 0, KumipuyoSeq());
    std::vector<EstimatedRensaInfoTree> enemyTree = HandTree::makeTree(2, cf2, PuyoSet(), 0, KumipuyoSeq());

    EXPECT_LT(0, HandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0));

#if 0
    for (const auto& t : myTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    for (const auto& t : enemyTree)
        t.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    cout << HandTree::eval(myTree, 0, 0, 0, enemyTree, 0, 0, 0) << endl;
#endif
}
