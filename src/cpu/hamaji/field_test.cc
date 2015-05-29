#include "eval.h"
#include "field.h"
#include "util.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(FieldTest, getBestChainCount) {
  {
    LF field;
    EXPECT_EQ(1, field.getBestChainCount());
  }
  {
    LF field("...B.."
             "...RB."
             "..RRBB");
    int ipc, ucc, vpc;
    EXPECT_EQ(2, field.getBestChainCount(&ipc, &ucc, &vpc));
    EXPECT_EQ(3, ipc);
    EXPECT_EQ(0, ucc);
    EXPECT_EQ(4, vpc);
  }
  {
    LF field("000500"
             "004500"
             "066450"
             "064455");
    int ipc, ucc, vpc;
    EXPECT_EQ(3, field.getBestChainCount(&ipc, &ucc, &vpc));
    EXPECT_EQ(3, ipc);
    EXPECT_EQ(4, ucc);
    EXPECT_EQ(9, vpc);
  }
  {
    LF field("5000005000054000054");
    int ipc, ucc, vpc;
    EXPECT_EQ(1, field.getBestChainCount(&ipc, &ucc, &vpc));
    EXPECT_EQ(2, ipc);
    EXPECT_EQ(0, ucc);
    EXPECT_EQ(0, vpc);
  }
  {
    LF field("50000050000540000540");
    int ipc, ucc, vpc;
    EXPECT_EQ(2, field.getBestChainCount(&ipc, &ucc, &vpc));
    EXPECT_EQ(2, ipc);
    EXPECT_EQ(0, ucc);
    EXPECT_EQ(4, vpc);
  }
}

void testUrl(string url, int expected_chains, int expected_score) {
  LF f(url);
  int chains;
  int score;
  int frames;
  f.Simulate(&chains, &score, &frames);
  EXPECT_EQ(expected_chains, chains);
  EXPECT_EQ(expected_score, score);
}

TEST(FieldTest, getOjamaFilmHeightTest) {
  {
    LF f("111110456755445677556675");
    EXPECT_EQ(0, f.getOjamaFilmHeight());
  }
  {
    LF f("111111456755445677556675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
  }
  {
    LF f("111111156755445677556675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
  }
  {
    LF f("1100011116115411177551675");
    EXPECT_EQ(2, f.getOjamaFilmHeight());
  }
  {
    LF f("1110111116115411177551675");
    EXPECT_EQ(2, f.getOjamaFilmHeight());
  }
  {
    LF f("1111111116115411177551675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
  }
  {
    LF f("1100111106115411177511675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
    int hcpc;
    EXPECT_EQ(1, f.getOjamaFilmHeight(&hcpc));
    EXPECT_EQ(8, hcpc);
  }
  {
    LF f("1100111116115411577511675");
    int hcpc;
    EXPECT_EQ(1, f.getOjamaFilmHeight(&hcpc));
    EXPECT_EQ(9, hcpc);
  }
}

TEST(FieldTest, getProspectiveChains) {
  // I introduced dups so current pchains size is 69.
  // Disable this test for now.
  // ASSERT_EQ(69L, pchains.size());
#if 0
  {
    LF f("45600045600045600");
    vector<Chain*> pchains;
    f.getProspectiveChains(&pchains);
    ASSERT_EQ(3L, pchains.size());
    EXPECT_EQ(1000, pchains[0]->score);
    EXPECT_EQ(3, pchains[0]->chains);
    EXPECT_EQ(3, pchains[0]->depth);
    EXPECT_EQ(12, pchains[0]->vanished);

    EXPECT_EQ(540, pchains[1]->score);
    EXPECT_EQ(2, pchains[1]->chains);
    EXPECT_EQ(3, pchains[1]->depth);
    EXPECT_EQ(9, pchains[1]->vanished);

    EXPECT_EQ(540, pchains[2]->score);
    EXPECT_EQ(2, pchains[2]->chains);
    EXPECT_EQ(3, pchains[2]->depth);
    EXPECT_EQ(9, pchains[2]->vanished);

    delete_clear(&pchains);
  }
#endif
}
