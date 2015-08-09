#include "decision_book.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

static const char TEST_BOOK[] =
    "[[book]]\n"
    "field = []\n"
    "AAAA = [3, 2]\n"
    "AAAB = [2, 3]\n"
    "AABB = [3, 3]\n"
    "ABAA = [3, 2]\n"
    "ABAB = [2, 2]\n"
    "ABAC = [3, 2]\n"
    "\n"
    "[[book]]\n"
    "field = [\n"
    "    \"..A...\",\n"
    "    \"..A...\"\n"
    "]\n"
    "AAAA = [5, 2]\n"
    "AAAB = [3, 0]\n"
    "AAAC = [3, 0]\n"
    "AABB = [3, 0]\n"
    "AABC = [3, 0]\n"
    "\n"
    "[[book]]\n"
    "field = [\n"
    "    \".AA...\"\n"
    "]\n"
    "BBCC = [3, 3]\n"
    "\n"
    "[[book]]\n"
    "field = [\n"
    "    \".B....\",\n"
    "    \".A....\"\n"
    "]\n"
    "ABAB = [3, 0]\n";

TEST(DecisionBookTest, nextDecision1)
{
    DecisionBook book;
    ASSERT_TRUE(book.loadFromString(TEST_BOOK));

    CoreField cf;
    KumipuyoSeq seq("RRRRRRGG");

    EXPECT_EQ(Decision(3, 2), book.nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 2), seq.front());
    seq.dropFront();

    EXPECT_EQ(Decision(5, 2), book.nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(5, 2), seq.front());
    seq.dropFront();

    EXPECT_FALSE(book.nextDecision(cf, seq).isValid());
}

TEST(DecisionBookTest, nextDecision2)
{
    DecisionBook book;
    ASSERT_TRUE(book.loadFromString(TEST_BOOK));

    CoreField cf;
    KumipuyoSeq seq("RRBBGG");

    EXPECT_EQ(Decision(3, 3), book.nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 3), seq.front());
    seq.dropFront();

    EXPECT_EQ(Decision(3, 3), book.nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 3), seq.front());
    seq.dropFront();
}

TEST(DecisionBookTest, nextDecision3)
{
    DecisionBook book;
    ASSERT_TRUE(book.loadFromString(TEST_BOOK));

    CoreField cf;
    KumipuyoSeq seq("RBRBBR");

    EXPECT_EQ(Decision(2, 2), book.nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(2, 2), seq.front());
    seq.dropFront();

    EXPECT_EQ(Decision(3, 2), book.nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 2), seq.front());
    seq.dropFront();
}
