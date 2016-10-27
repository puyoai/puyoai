#include "decision_book.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

namespace {

const char TEST_BOOK[] =
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
    "AA = [3, 0]\n"
    "AAAA = [5, 2]\n"
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
}  // namespace

class DecisionBookTest : public testing::Test {
 public:
  void SetUp() override
  {
    ASSERT_TRUE(m_book.loadFromString(TEST_BOOK));
  }

  DecisionBook& book() { return m_book; }

 private:
  DecisionBook m_book;
};

TEST_F(DecisionBookTest, nextDecisionNotInBook)
{
    CoreField cf;
    KumipuyoSeq seq("RRBG");

    EXPECT_FALSE(book().nextDecision(cf, seq).isValid());
}

TEST_F(DecisionBookTest, nextDecisionWithOneTsumo)
{
    CoreField cf;
    KumipuyoSeq seq("RRRRGG");

    EXPECT_EQ(Decision(3, 2), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 2), seq.front());
    seq.dropFront();

    // We have no entry to match with "RRGG", so "RR" is used.
    EXPECT_EQ(Decision(3, 0), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 0), seq.front());
    seq.dropFront();
}

TEST_F(DecisionBookTest, nextDecision1)
{
    CoreField cf;
    KumipuyoSeq seq("RRRRRRGG");

    EXPECT_EQ(Decision(3, 2), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 2), seq.front());
    seq.dropFront();

    // 2 Tsumo's control is prioritized to 1 Tsumo's control.
    EXPECT_EQ(Decision(5, 2), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(5, 2), seq.front());
    seq.dropFront();

    EXPECT_FALSE(book().nextDecision(cf, seq).isValid());
}

TEST_F(DecisionBookTest, nextDecision2)
{
    CoreField cf;
    KumipuyoSeq seq("RRBBGG");

    EXPECT_EQ(Decision(3, 3), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 3), seq.front());
    seq.dropFront();

    EXPECT_EQ(Decision(3, 3), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 3), seq.front());
    seq.dropFront();
}

TEST_F(DecisionBookTest, nextDecision3)
{
    CoreField cf;
    KumipuyoSeq seq("RBRBBR");

    EXPECT_EQ(Decision(2, 2), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(2, 2), seq.front());
    seq.dropFront();

    EXPECT_EQ(Decision(3, 2), book().nextDecision(cf, seq));
    cf.dropKumipuyo(Decision(3, 2), seq.front());
    seq.dropFront();
}
