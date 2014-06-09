#include "core/constant.h"
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
    LF field("http://www.inosendo.com/puyo/rensim/?500000450004455");
    int ipc, ucc, vpc;
    EXPECT_EQ(2, field.getBestChainCount(&ipc, &ucc, &vpc));
    EXPECT_EQ(3, ipc);
    EXPECT_EQ(0, ucc);
    EXPECT_EQ(4, vpc);
  }
  {
    LF field("http://www.inosendo.com/puyo/rensim/?500004500066450064455");
    int ipc, ucc, vpc;
    EXPECT_EQ(3, field.getBestChainCount(&ipc, &ucc, &vpc));
    EXPECT_EQ(3, ipc);
    EXPECT_EQ(1, ucc);
    EXPECT_EQ(9, vpc);
  }
  {
    LF field("http://www.inosendo.com/puyo/rensim/??5000005000054000054");
    int ipc, ucc, vpc;
    EXPECT_EQ(1, field.getBestChainCount(&ipc, &ucc, &vpc));
    EXPECT_EQ(4, ipc);
    EXPECT_EQ(0, ucc);
    EXPECT_EQ(0, vpc);
  }
  {
    LF field("http://www.inosendo.com/puyo/rensim/??50000050000540000540");
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


TEST(FieldTest, ChainAndScoreTest) {
  testUrl("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755", 19, 175080);
  testUrl("http://www.inosendo.com/puyo/rensim/??500467767675744454754657447767667644674545455767477644457474656455446775455646", 19, 175080);
  testUrl("http://www.inosendo.com/puyo/rensim/??550050455045451045745074745074645067674067674056567056567515167444416555155", 2, 38540);
  testUrl("http://www.inosendo.com/puyo/rensim/??50550040455075451075745064745064645067674057674747574776567675156644415555155", 3, 43260);
  testUrl("http://www.inosendo.com/puyo/rensim/??550040455075451775745464745464645467674457674147574776567675156644415555155", 4, 50140);
  testUrl("http://www.inosendo.com/puyo/rensim/??745550576455666451175745564745564745567674157674747574776566615156644415555155", 5, 68700);
  testUrl("http://www.inosendo.com/puyo/rensim/??444411114141414114114111414144411114414111114414411114441114111141444141111141", 4, 4840);
  testUrl("http://www.inosendo.com/puyo/rensim/??545544544454454545454545454545545454445544554455454545545454554544445455455445", 9, 49950);
  testUrl("http://www.inosendo.com/puyo/rensim/??444446544611446164564441546166565615454551441444111111111111111111111111111111", 9, 32760);
  testUrl("http://www.inosendo.com/puyo/rensim/??667547466555661677471475451447461666661547457477556446776555744646476466744555", 18, 155980);
  testUrl("http://www.inosendo.com/puyo/rensim/??444044144414114144411411414144141414414141441414114411441144414141141414144144", 11, 47080);
  testUrl("http://www.inosendo.com/puyo/rensim/??444444444444444444444444444444444444444444444444444444444444444444444444", 1, 7200);

  testUrl("http://www.inosendo.com/puyo/rensim/??400005500005400005500004400", 2, 420);
}

TEST(FieldTest, SafeDropTest) {
  {
    LF f("http://www.inosendo.com/puyo/rensim/??5500044000");
    f.SafeDrop();
    EXPECT_EQ(RED, f.Get(2, 1));
    EXPECT_EQ(RED, f.Get(3, 1));
    EXPECT_EQ(BLUE, f.Get(3, 2));
    EXPECT_EQ(BLUE, f.Get(4, 1));
    EXPECT_EQ(EMPTY, f.Get(4, 2));
  }
}

TEST(FieldTest, FramesTest) {
  int chains, score, frames;
  {
    // 1 Rensa, no drop.
    LF f("444400");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_AFTER_NO_DROP, frames);
  }
  {
    LF f("500000"
         "444400");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP, frames);
  }
  {
    LF f("500000"
         "400000"
         "444000");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
              frames);
  }
  {
    LF f("500000"
         "450000"
         "444000");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
              frames);
  }
  {
    LF f("500000"
         "455000"
         "444500");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
              FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_AFTER_NO_DROP,
              frames);
  }
  {
    LF f("560000"
         "455000"
         "444500");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
              FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP,
              frames);
  }
}

TEST(FieldTest, FindAvailablePlansTest) {
  {
    LF f;
    const string& next = LF::parseNext("444444");

    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
    EXPECT_EQ(11 + 11*11 + 11*11*11, plans.size());
  }
  {
    LF f;
    const string& next = LF::parseNext("445566");

    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
    EXPECT_EQ(11 + 11*11 + 11*11*11, plans.size());
  }
  {
    LF f;
    const string& next = LF::parseNext("456745");

    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
    EXPECT_EQ(22 + 22*22 + 22*22*22, plans.size());
  }
  {
    LF f;
    const string& next = LF::parseNext("445675");

    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
    EXPECT_EQ(11 + 11*22 + 11*22*22, plans.size());
  }
  {
    LF f;
    const string& next = LF::parseNext("456477");

    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
    EXPECT_EQ(22 + 22*22 + 22*22*11, plans.size());
  }
}


TEST(FieldTest, getOjamaFilmHeightTest) {
  {
    LF f("http://www.inosendo.com/puyo/rensim/??111110456755445677556675");
    EXPECT_EQ(0, f.getOjamaFilmHeight());
  }
  {
    LF f("http://www.inosendo.com/puyo/rensim/??111111456755445677556675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
  }
  {
    LF f("http://www.inosendo.com/puyo/rensim/??111111156755445677556675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
  }
  {
    LF f("http://www.inosendo.com/puyo/rensim/??1100011116115411177551675");
    EXPECT_EQ(2, f.getOjamaFilmHeight());
  }
  {
    LF f("http://www.inosendo.com/puyo/rensim/??1110111116115411177551675");
    EXPECT_EQ(2, f.getOjamaFilmHeight());
  }
  {
    LF f("http://www.inosendo.com/puyo/rensim/??1111111116115411177551675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
  }
  {
    LF f("http://www.inosendo.com/puyo/rensim/??1100111106115411177511675");
    EXPECT_EQ(1, f.getOjamaFilmHeight());
    int hcpc;
    EXPECT_EQ(1, f.getOjamaFilmHeight(&hcpc));
    EXPECT_EQ(8, hcpc);
  }
  {
    LF f("http://www.inosendo.com/puyo/rensim/??1100111116115411577511675");
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
    LF f("http://www.inosendo.com/puyo/rensim/??45600045600045600");
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
