#include "capture/somagic_analyzer.h"

#include <algorithm>
#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <SDL_image.h>
#include "core/next_puyo.h"
#include "core/real_color.h"
#include "gui/unique_sdl_surface.h"

using namespace std;

DECLARE_string(testdata_dir);

class SomagicAnalyzerTest : public testing::Test {
protected:
    unique_ptr<AnalyzerResult> analyze(const string& imgFilename)
    {
        string filename = FLAGS_testdata_dir + imgFilename;
        UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(filename.c_str())));
        CHECK(surf.get()) << "Failed to load " << filename;

        SomagicAnalyzer analyzer;
        return analyzer.analyze(surf.get(), deque<unique_ptr<AnalyzerResult>>());
    }

    deque<unique_ptr<AnalyzerResult>> analyzeMultipleFrames(const vector<string>& imgFilenames,
                                                            bool userPlayable[2])
    {
        deque<unique_ptr<AnalyzerResult>> results;

        SomagicAnalyzer analyzer;
        bool firstResult = true;

        for (const auto& imgFilename : imgFilenames) {
            string filename = FLAGS_testdata_dir + imgFilename;
            auto surface = makeUniqueSDLSurface(IMG_Load(filename.c_str()));
            CHECK(surface.get()) << "Failed to load "<< filename;
            auto r = analyzer.analyze(surface.get(), results);
            if (firstResult) {
                r->mutablePlayerResult(0)->userState.playable = userPlayable[0];
                r->mutablePlayerResult(1)->userState.playable = userPlayable[1];
                firstResult = false;
            }
            results.push_front(move(r));
        }

        reverse(results.begin(), results.end());
        return results;
    }
};

TEST_F(SomagicAnalyzerTest, AnalyzeNormalField1)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/field-normal1.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(1, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(2, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(3, 1));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->realColor(4, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(5, 1));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(6, 1));

    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->realColor(1, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->realColor(2, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->realColor(3, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(4, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(5, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(6, 2));

    EXPECT_EQ(RealColor::RC_GREEN,  r->playerResult(0)->realColor(1, 4));
    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->realColor(2, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(3, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(4, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(5, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(6, 4));
}

TEST_F(SomagicAnalyzerTest, AnalyzeFieldNormal6)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/field-normal6.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(1, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(2, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(3, 1));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(4, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(5, 1));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(6, 1));

    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->realColor(1, 2));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->realColor(2, 2));
    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->realColor(3, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(4, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(5, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->realColor(6, 2));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase1)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case1.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase2)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case2.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase3)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case3.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase4)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case4.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, WnextDetection)
{
    struct Testcase {
        int playerId;
        RealColor rc;
        const char* name;
        int maxFrame;
    };

    Testcase testcases[] = {
        { 0, RealColor::RC_BLUE,   "blue",   8 },
        { 0, RealColor::RC_GREEN,  "green",  8 },
        { 0, RealColor::RC_PURPLE, "purple", 6 },
        { 0, RealColor::RC_RED,    "red",    8 },
        { 0, RealColor::RC_YELLOW, "yellow", 7 },
        { 1, RealColor::RC_BLUE,   "blue",   6 },
        { 1, RealColor::RC_GREEN,  "green",  8 },
        { 1, RealColor::RC_PURPLE, "purple", 8 },
        { 1, RealColor::RC_RED,    "red",    9 },
        { 1, RealColor::RC_YELLOW, "yellow", 8 },
    };

    for (int i = 0; i < sizeof(testcases) / sizeof(testcases[0]); ++i) {
        for (int id = 1; id <= testcases[i].maxFrame; ++id) {
            char buf[80];
            sprintf(buf, "/somagic/wnext/%s-%dp-case%d.png", testcases[i].name, testcases[i].playerId + 1, id);
            unique_ptr<AnalyzerResult> r = analyze(buf);

            EXPECT_EQ(testcases[i].rc, r->playerResult(testcases[i].playerId)->realColor(NextPuyoPosition::NEXT2_AXIS))
                << " Player " << testcases[i].playerId
                << " Testcase " << testcases[i].name
                << " Case " << id;
        }
    }
}

TEST_F(SomagicAnalyzerTest, LevelSelect1)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/level-select1.png");

    EXPECT_EQ(CaptureGameState::LEVEL_SELECT, r->state());
}

TEST_F(SomagicAnalyzerTest, LevelSelect2)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/level-select2.png");

    EXPECT_EQ(CaptureGameState::LEVEL_SELECT, r->state());
}

TEST_F(SomagicAnalyzerTest, GameFinished)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/game-finished1.png");

    EXPECT_EQ(CaptureGameState::FINISHED, r->state());
}

TEST_F(SomagicAnalyzerTest, NextArrival)
{
    vector<string> images;
    for (int i = 0; i <= 24; ++i) {
        char buf[80];
        sprintf(buf, "/somagic/next-arrival/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    EXPECT_TRUE(rs[0]->playerResult(0)->userState.playable);
    // Next disappears here.
    // Either 13 or 14 should not be playable.
    EXPECT_FALSE(rs[13]->playerResult(0)->userState.playable && rs[14]->playerResult(0)->userState.playable);
    // Then controllable now.
    EXPECT_TRUE(rs[16]->playerResult(0)->userState.playable);
}

TEST_F(SomagicAnalyzerTest, Vanishing)
{
    vector<string> images;
    for (int i = 0; i <= 15; ++i) {
        char buf[80];
        sprintf(buf, "/somagic/vanishing/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    for (const auto& r : rs) {
        EXPECT_EQ(CaptureGameState::PLAYING, r->state());
    }

    EXPECT_TRUE(rs[0]->playerResult(0)->userState.playable);
    EXPECT_TRUE(rs[10]->playerResult(0)->userState.playable);
    EXPECT_TRUE(rs[11]->playerResult(0)->userState.playable);
    EXPECT_TRUE(rs[12]->playerResult(0)->userState.playable);

    // vanishing should be detected here.
    // TODO(mayah): We might be able to detect in the previous frame?
    EXPECT_FALSE(rs[13]->playerResult(0)->userState.playable);
}

TEST_F(SomagicAnalyzerTest, OjamaDrop)
{
    vector<string> images;
    for (int i = 0; i <= 38; ++i) {
        char buf[80];
        sprintf(buf, "/somagic/ojama-drop-2p/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    for (const auto& r : rs) {
        EXPECT_EQ(CaptureGameState::PLAYING, r->state());
    }

    EXPECT_TRUE(rs[0]->playerResult(1)->userState.playable);
    EXPECT_TRUE(rs[1]->playerResult(1)->userState.playable);
    EXPECT_TRUE(rs[2]->playerResult(1)->userState.playable);
    EXPECT_TRUE(rs[3]->playerResult(1)->userState.playable);
    EXPECT_TRUE(rs[4]->playerResult(1)->userState.playable);
    EXPECT_TRUE(rs[5]->playerResult(1)->userState.playable);
    // Around here, analyzer can detect the puyo is grounded, since we can see ojama puyo.
    EXPECT_FALSE(rs[8]->playerResult(1)->userState.playable);
    EXPECT_FALSE(rs[31]->playerResult(1)->userState.playable);
    // Then, next puyo will disappear.
    EXPECT_TRUE(rs[37]->playerResult(1)->userState.playable);
}

TEST_F(SomagicAnalyzerTest, GameStart)
{
    vector<string> images;
    for (int i = 0; i <= 61; ++i) {
        char buf[80];
        sprintf(buf, "/somagic/game-start/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { false, false };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    // Frame 0-2 should be considered as LEVEL_SELECT.
    for (int i = 0; i <= 2; ++i)
        EXPECT_EQ(CaptureGameState::LEVEL_SELECT, rs[i]->state());
    // At least frame 8, game state should be PLAYING.
    for (int i = 8; i <= 61; ++i)
        EXPECT_EQ(CaptureGameState::PLAYING, rs[i]->state());

    struct {
        int frame;
        const char* player1;
        const char* player2;
    } expectations[] = {
        {  0, "  ----", "  ----" },  // Frame 0-2 are level select. On level select, current puyo should be empty.
        {  1, "  ----", "  ----" },  // Also, we will use several frames to make NEXT stabilize.
        {  2, "  ----", "  ----" },
        {  3, "  BBPP", "  BBPP" },
        {  4, "  BBPP", "  BBPP" },
        {  5, "  BBPP", "  BBPP" },
        {  6, "  BBPP", "  BBPP" },
        {  7, "  BBPP", "  BBPP" },
        {  8, "  BBPP", "  BBPP" },
        {  9, "  BBPP", "  BBPP" },
        { 10, "  BBPP", "  BBPP" },
        { 11, "  BBPP", "  BBPP" },  // Player2: Green character is on NEXT2, however, analyzer should say NEXT2 is still PP.
        { 12, "  BBPP", "  BBPP" },
        { 13, "  BBPP", "  BBPP" },
        { 14, "  BBPP", "  BBPP" },
        { 15, "  BBPP", "  BBPP" },
        { 16, "  BBPP", "  BBPP" },
        { 17, "  BBPP", "  BBPP" },
        { 18, "  BBPP", "  BBPP" },
        { 19, "  BBPP", "  BBPP" },
        { 20, "  BBPP", "  BBPP" },
        { 21, "  BBPP", "  BBPP" },
        { 22, "------", "------" },
        { 23, "------", "------" },
        { 24, "------", "------" },
        { 25, "------", "------" },
        { 26, "------", "------" },
        { 27, "------", "------" },
        { 28, "------", "BBPP  " },
        { 29, "BBPP  ", "BBPP  " },
        { 30, "BBPP  ", "BBPP  " },
        { 31, "------", "BBPP--" },  // Player 2, frame 31: Since some stars are located on NEXT2_AXIS,
        { 32, "BBPP--", "BBPP--" },  // analyzer might consider the axis color is YELLOW.
        { 33, "BBPPBB", "BBPPBB" },  // However, we should fix in frame33 at least.
        { 34, "BBPPBB", "BBPPBB" },
        { 35, "BBPPBB", "BBPPBB" },
        { 36, "BBPPBB", "BBPPBB" },
        { 37, "BBPPBB", "BBPPBB" },
        { 38, "BBPPBB", "BBPPBB" },
        { 39, "BBPPBB", "BBPPBB" },
        { 40, "BBPPBB", "BBPPBB" },
        { 41, "BBPPBB", "BBPPBB" },
        { 42, "BBPPBB", "BBPPBB" },
        { 43, "BBPPBB", "BBPPBB" },
        { 44, "BBPPBB", "BBPPBB" },
        { 45, "BBPPBB", "BBPPBB" },
        { 46, "BBPPBB", "BBPPBB" },
        { 47, "BBPPBB", "BBPPBB" },
        { 48, "PPBB  ", "BBPPBB" },  // Player 1, frame 48: next1 will disappear. so next2 is moved to next1 on that time.
        { 49, "PPBB  ", "BBPPBB" },
        { 50, "PPBB  ", "BBPPBB" },
        { 51, "PPBB  ", "BBPPBB" },
        { 52, "PPBB  ", "BBPPBB" },
        { 53, "PPBB  ", "BBPPBB" },
        { 54, "PPBB  ", "BBPPBB" },
        { 55, "PPBB  ", "BBPPBB" },
        { 56, "PPBB  ", "BBPPBB" },
        { 57, "PPBB  ", "BBPPBB" },
        { 58, "PPBB  ", "BBPPBB" },
        { 59, "PPBB  ", "BBPPBB" },
        { 60, "PPBB--", "BBPPBB" },  // Player 1, frame 60: Yellow will appeaer. But we will need 3 frames to detect it.
        { 61, "PPBB--", "BBPPBB" },
    };

    for (size_t i = 0; i < sizeof(expectations) / sizeof(expectations[0]); ++i) {
        const PlayerAnalyzerResult* p1 = rs[expectations[i].frame]->playerResult(0);
        const PlayerAnalyzerResult* p2 = rs[expectations[i].frame]->playerResult(1);
        const char* c1 = expectations[i].player1;
        const char* c2 = expectations[i].player2;

        if (c1[0] != '-') EXPECT_EQ(realColorOf(c1[0]), p1->realColor(NextPuyoPosition::CURRENT_AXIS)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[1] != '-') EXPECT_EQ(realColorOf(c1[1]), p1->realColor(NextPuyoPosition::CURRENT_CHILD)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[2] != '-') EXPECT_EQ(realColorOf(c1[2]), p1->realColor(NextPuyoPosition::NEXT1_AXIS)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[3] != '-') EXPECT_EQ(realColorOf(c1[3]), p1->realColor(NextPuyoPosition::NEXT1_CHILD)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[4] != '-') EXPECT_EQ(realColorOf(c1[4]), p1->realColor(NextPuyoPosition::NEXT2_AXIS)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[5] != '-') EXPECT_EQ(realColorOf(c1[5]), p1->realColor(NextPuyoPosition::NEXT2_CHILD)) << "Player1 : Frame " << expectations[i].frame;

        if (c2[0] != '-') EXPECT_EQ(realColorOf(c2[0]), p2->realColor(NextPuyoPosition::CURRENT_AXIS)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[1] != '-') EXPECT_EQ(realColorOf(c2[1]), p2->realColor(NextPuyoPosition::CURRENT_CHILD)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[2] != '-') EXPECT_EQ(realColorOf(c2[2]), p2->realColor(NextPuyoPosition::NEXT1_AXIS)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[3] != '-') EXPECT_EQ(realColorOf(c2[3]), p2->realColor(NextPuyoPosition::NEXT1_CHILD)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[4] != '-') EXPECT_EQ(realColorOf(c2[4]), p2->realColor(NextPuyoPosition::NEXT2_AXIS)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[5] != '-') EXPECT_EQ(realColorOf(c2[5]), p2->realColor(NextPuyoPosition::NEXT2_CHILD)) << "Player2 : Frame " << expectations[i].frame;
    }

    // ----------------------------------------------------------------------

    for (int i = 21; i <= 31; ++i) {
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->realColor(NextPuyoPosition::NEXT1_AXIS))
            << "Player 1: Frame" << i << ": NEXT1_AXIS";
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->realColor(NextPuyoPosition::NEXT1_CHILD))
            << "Player 1: Frame" << i << ": NEXT1_CHILD";
    }
    for (int i = 45; i <= 59; ++i) {
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->realColor(NextPuyoPosition::NEXT1_AXIS))
            << "Player 1: Frame" << i << ": NEXT1_AXIS";
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->realColor(NextPuyoPosition::NEXT1_CHILD))
            << "Player 1: Frame" << i << ": NEXT1_CHILD";
    }

    // In any frame, ojama is not dropping.
    for (int i = 0; i <= 61; ++i) {
        EXPECT_FALSE(rs[i]->playerResult(0)->userState.ojamaDropped);
        EXPECT_FALSE(rs[i]->playerResult(1)->userState.ojamaDropped);
    }
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    google::ParseCommandLineFlags(&argc, &argv, true);

    SDL_Init(SDL_INIT_VIDEO);
    int r = RUN_ALL_TESTS();
    SDL_Quit();
    return r;
}
