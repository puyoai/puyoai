#include "rensa_hand_tree.h"

#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

RensaHand makePlainRensaHand(int chains)
{
    RensaResult rensaResult(chains,
                            ACCUMULATED_RENSA_SCORE[chains],
                            chains * NUM_FRAMES_OF_ONE_RENSA,
                            false);

    RensaCoefResult coefResult;
    for (int i = 1; i <= chains; ++i) {
        coefResult.setCoef(i, 4, 0, 0);
    }

    return RensaHand(IgnitionRensaResult(rensaResult, 0, NUM_FRAMES_OF_ONE_HAND), coefResult);
}

class RensaHandTreeTest : public testing::Test {
public:
    RensaHandTreeTest()
    {
        PuyoPossibility::initialize();
    }
};

TEST(RensaHandTreeTest, eval_empty)
{
    RensaHandTree empty;
    EXPECT_EQ(0, RensaHandTree::eval(empty, 0, 0, 0, 0, empty, 0, 0, 0, 0));
}

TEST(RensaHandTreeTest, eval_5rensa)
{
    std::vector<RensaHandEdge> myEdges { RensaHandEdge(makePlainRensaHand(5), RensaHandTree()) };
    std::vector<RensaHandNode> myNodes { RensaHandNode(myEdges) };
    RensaHandTree myTree(myNodes);

    const RensaHandTree enemyTree;

    int s = RensaHandTree::eval(myTree, 0, 0, 0, 0, enemyTree, 0, 0, 0, 0);
    cout << s << endl;

    // 5rensa = 69, 3rensa = 14, 2rensa = 5
    // 64 instead of 69?
    EXPECT_EQ(69, RensaHandTree::eval(myTree, 0, 0, 0, 0, enemyTree, 0, 0, 0, 0));
}

TEST(RensaHandTreeTest, eval_saisoku)
{
    // 1P has 10 rensa.
    std::vector<RensaHandEdge> myEdges { RensaHandEdge(makePlainRensaHand(8), RensaHandTree()) };
    std::vector<RensaHandNode> myNodes { RensaHandNode(myEdges) };
    RensaHandTree myTree(myNodes);

    // 2P has 11 rensa.
    std::vector<RensaHandEdge> enemyEdges { RensaHandEdge(makePlainRensaHand(11), RensaHandTree()) };
    std::vector<RensaHandNode> enemyNodes { RensaHandNode(enemyEdges) };
    RensaHandTree enemyTree(enemyNodes);

    // Eval after 1P has fired 2-double.
    int s = RensaHandTree::eval(myTree, 2 * NUM_FRAMES_OF_ONE_RENSA, 0, 0, 0,
                                enemyTree, 0, 0, 15, 2 * NUM_FRAMES_OF_ONE_RENSA);

    // TODO(mayah): Is this correct?
    EXPECT_LE(15, s);

#if 0
    cout << s << endl;
#endif
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
        "RBYGRO"
        "RBYGRO");

    RensaHandTree myTree = RensaHandTree::makeTree(2, cf1, PuyoSet(), 0, KumipuyoSeq("YYYY"));
    RensaHandTree enemyTree = RensaHandTree::makeTree(2, cf2, PuyoSet(), 0, KumipuyoSeq("YYGG"));

#if 0
    myTree.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    enemyTree.dump(0);
    cout << "----------------------------------------------------------------------" << endl;

    cout << RensaHandTree::eval(myTree, 0, 0, 0, 0, enemyTree, 0, 0, 0, 0) << endl;
#endif

    EXPECT_LT(0, RensaHandTree::eval(myTree, 0, 0, 0, 0, enemyTree, 0, 0, 0, 0));
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

    RensaHandTree myTree = RensaHandTree::makeTree(2, cf1, PuyoSet(), 0, KumipuyoSeq("BB"));
    RensaHandTree enemyTree = RensaHandTree::makeTree(2, cf2, PuyoSet(), 0, KumipuyoSeq());

    EXPECT_LT(0, RensaHandTree::eval(myTree, 0, 0, 0, 0, enemyTree, 0, 0, 0, 0));

#if 0
    myTree.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    enemyTree.dump(0);
    cout << "----------------------------------------------------------------------" << endl;
    cout << RensaHandTree::eval(myTree, 0, 0, 0, 0, enemyTree, 0, 0, 0, 0) << endl;
#endif
}
