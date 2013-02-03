#include <gtest/gtest.h>
#include <string>

#include "field.h"
#include "plan.h"

using namespace std;

void testUrl(string url, int expected_chains, int expected_score) {
  Field f(url);
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
}

TEST(FieldTest, FramesTest) {
  int chains, score, frames;
  {
    // 1 Rensa, no drop.
    Field f("444400");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP, frames);
  }
  {
    Field f("500000"
            "444400");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP, frames);
  }
  {
    Field f("500000"
            "400000"
            "444000");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
              frames);
  }
  {
    Field f("500000"
            "450000"
            "444000");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
              frames);
  }
  {
    Field f("500000"
            "455000"
            "444500");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
              FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP,
              frames);
  }
  {
    Field f("560000"
            "455000"
            "444500");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
              FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP,
              frames);
  }
}

TEST(FieldTest, FindAvailablePlansTest) {
  {
    Field f;
    f.SetColorSequence("444444");

    vector<Plan> plans;
    f.FindAvailablePlans(&plans);
    EXPECT_EQ(11 + 11*11 + 11*11*11, plans.size());
  }
  {
    Field f;
    f.SetColorSequence("445566");

    vector<Plan> plans;
    f.FindAvailablePlans(&plans);
    EXPECT_EQ(11 + 11*11 + 11*11*11, plans.size());
  }
  {
    Field f;
    f.SetColorSequence("456745");

    vector<Plan> plans;
    f.FindAvailablePlans(&plans);
    EXPECT_EQ(22 + 22*22 + 22*22*22, plans.size());
  }
  {
    Field f;
    f.SetColorSequence("445675");

    vector<Plan> plans;
    f.FindAvailablePlans(&plans);
    EXPECT_EQ(11 + 11*22 + 11*22*22, plans.size());
  }
  {
    Field f;
    f.SetColorSequence("456477");

    vector<Plan> plans;
    f.FindAvailablePlans(&plans);
    EXPECT_EQ(22 + 22*22 + 22*22*11, plans.size());
  }
}
