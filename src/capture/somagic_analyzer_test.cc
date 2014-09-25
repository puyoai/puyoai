#include "capture/somagic_analyzer.h"

#include <algorithm>
#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <SDL_image.h>

#include "capture/color.h"
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
        return analyzer.analyze(surf.get(), nullptr, deque<unique_ptr<AnalyzerResult>>());
    }

    deque<unique_ptr<AnalyzerResult>> analyzeMultipleFrames(const vector<string>& imgFilenames,
                                                            bool userPlayable[2])
    {
        deque<unique_ptr<AnalyzerResult>> results;

        SomagicAnalyzer analyzer;
        bool firstResult = true;
        UniqueSDLSurface prev(emptyUniqueSDLSurface());

        for (const auto& imgFilename : imgFilenames) {
            string filename = FLAGS_testdata_dir + imgFilename;
            auto surface = makeUniqueSDLSurface(IMG_Load(filename.c_str()));
            CHECK(surface.get()) << "Failed to load "<< filename;
            auto r = analyzer.analyze(surface.get(), prev.get(), results);
            if (firstResult) {
                r->mutablePlayerResult(0)->userState.playable = userPlayable[0];
                r->mutablePlayerResult(1)->userState.playable = userPlayable[1];
                firstResult = false;
            }
            results.push_front(move(r));
            prev = move(surface);
        }

        reverse(results.begin(), results.end());
        return results;
    }
};

TEST_F(SomagicAnalyzerTest, estimateRealColor)
{
    // These RGB values are taken from the real example.
    struct Testcase {
        RealColor expected;
        RGB rgb;
    } testcases[] = {
        { RealColor::RC_EMPTY,  RGB( 71,  62,   2) },
        { RealColor::RC_EMPTY,  RGB( 71,  64,   0) },
        { RealColor::RC_EMPTY,  RGB( 35,  26,   0) },
        { RealColor::RC_OJAMA,  RGB(255, 255, 255) },
        { RealColor::RC_OJAMA,  RGB(156, 199, 177) },
        { RealColor::RC_OJAMA,  RGB(141, 187, 162) },
        { RealColor::RC_RED,    RGB(255,   0,   0) },
        { RealColor::RC_RED,    RGB(236, 134, 144) },
        { RealColor::RC_RED,    RGB(220, 162, 172) },
        { RealColor::RC_RED,    RGB(119,  59,  69) },
        { RealColor::RC_RED,    RGB(105,  29,  53) },
        { RealColor::RC_GREEN,  RGB(  0, 255,   0) },
        { RealColor::RC_GREEN,  RGB(161, 193, 122) },
        { RealColor::RC_GREEN,  RGB( 79, 200,  57) },
        { RealColor::RC_GREEN,  RGB( 25, 117,   6) },
        { RealColor::RC_GREEN,  RGB( 25,  95,   0) },
        { RealColor::RC_GREEN,  RGB( 19,  89,   5) },
        { RealColor::RC_GREEN,  RGB(  6,  73,   0) },
        { RealColor::RC_BLUE,   RGB(  0,   0, 255) },
        { RealColor::RC_BLUE,   RGB( 78, 165, 232) },
        { RealColor::RC_BLUE,   RGB( 31,  81, 170) },
        { RealColor::RC_BLUE,   RGB(114, 200, 235) },
        { RealColor::RC_BLUE,   RGB( 43,  71,  71) },
        { RealColor::RC_BLUE,   RGB( 52,  76,  76) },
        { RealColor::RC_YELLOW, RGB(255, 255,   0) },
        { RealColor::RC_YELLOW, RGB(177, 154,  64) },
        { RealColor::RC_YELLOW, RGB(200, 153,  53) },
        { RealColor::RC_PURPLE, RGB( 94,  21,  78) },
        { RealColor::RC_PURPLE, RGB( 77,  23,  35) },
        { RealColor::RC_PURPLE, RGB( 84,  45,  52) },
        { RealColor::RC_PURPLE, RGB(135,  34, 142) },
    };
    int size = ARRAY_SIZE(testcases);

    for (int i = 0; i < size; ++i) {
        EXPECT_EQ(testcases[i].expected, SomagicAnalyzer::estimateRealColor(testcases[i].rgb.toHSV()))
            << " expected=" << toString(testcases[i].expected)
            << " actual=" << toString(SomagicAnalyzer::estimateRealColor(testcases[i].rgb.toHSV()))
            << " RGB=" << testcases[i].rgb.toString()
            << " HSV=" << testcases[i].rgb.toHSV().toString();

    }
}

TEST_F(SomagicAnalyzerTest, analyzeNormalField1)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/field-normal1.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(1, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(2, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(3, 1));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(4, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(5, 1));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(6, 1));

    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->adjustedField.realColor(1, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(2, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(3, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(4, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(5, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(6, 2));

    EXPECT_EQ(RealColor::RC_GREEN,  r->playerResult(0)->adjustedField.realColor(1, 4));
    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->adjustedField.realColor(2, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(3, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(4, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(5, 4));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(6, 4));
}

TEST_F(SomagicAnalyzerTest, analyzeFieldNormal6)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/field-normal6.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(1, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(2, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(3, 1));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(4, 1));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(5, 1));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(6, 1));

    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->adjustedField.realColor(1, 2));
    EXPECT_EQ(RealColor::RC_BLUE,   r->playerResult(0)->adjustedField.realColor(2, 2));
    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->adjustedField.realColor(3, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(4, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(5, 2));
    EXPECT_EQ(RealColor::RC_EMPTY,  r->playerResult(0)->adjustedField.realColor(6, 2));
}

TEST_F(SomagicAnalyzerTest, analyzeFieldNormal7)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/field-normal7.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(2, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(4, 6));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(4, 7));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(4, 8));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(2, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(2, 3));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(3, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(3, 6));
}

TEST_F(SomagicAnalyzerTest, AnalyzeAnotherField1)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/field-another1.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(1, 1));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(1, 2));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(2, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(2, 2));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(2, 3));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(3, 3));
    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->adjustedField.realColor(3, 1));
    EXPECT_EQ(RealColor::RC_YELLOW, r->playerResult(0)->adjustedField.realColor(3, 2));
    EXPECT_EQ(RealColor::RC_GREEN,  r->playerResult(0)->adjustedField.realColor(3, 4));
    EXPECT_EQ(RealColor::RC_GREEN,  r->playerResult(0)->adjustedField.realColor(4, 1));
    EXPECT_EQ(RealColor::RC_GREEN,  r->playerResult(0)->adjustedField.realColor(4, 2));
    EXPECT_EQ(RealColor::RC_GREEN,  r->playerResult(0)->adjustedField.realColor(4, 3));

    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(1, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(2, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(2, 3));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(3, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(1)->adjustedField.realColor(4, 1));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(1)->adjustedField.realColor(5, 1));
}

TEST_F(SomagicAnalyzerTest, AnalyzeAnotherField2)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/field-another2.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(2, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(3, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(4, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(4, 10));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(0)->adjustedField.realColor(4, 11));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(4, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(4, 3));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(4, 4));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(0)->adjustedField.realColor(4, 7));

    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(2, 1));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(2, 2));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(4, 3));
    EXPECT_EQ(RealColor::RC_RED,    r->playerResult(1)->adjustedField.realColor(6, 6));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(1)->adjustedField.realColor(1, 1));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(1)->adjustedField.realColor(1, 2));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(1)->adjustedField.realColor(1, 3));
    EXPECT_EQ(RealColor::RC_PURPLE, r->playerResult(1)->adjustedField.realColor(2, 4));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase1)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case1.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->adjustedField.realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase2)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case2.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->adjustedField.realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase3)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case3.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->adjustedField.realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, OjamaDetectionCase4)
{
    unique_ptr<AnalyzerResult> r = analyze("/somagic/ojama-detection/case4.png");

    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(1, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(2, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(3, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(4, 6));
    EXPECT_EQ(RealColor::RC_EMPTY, r->playerResult(0)->adjustedField.realColor(5, 6));
    EXPECT_EQ(RealColor::RC_OJAMA, r->playerResult(0)->adjustedField.realColor(6, 6));
}

TEST_F(SomagicAnalyzerTest, WnextDetection)
{
    struct Testcase {
        int playerId;
        RealColor rc;
        const char* name;
        int maxFrame;
    };

    vector<Testcase> testcases = {
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

    for (size_t i = 0; i < testcases.size(); ++i) {
        for (int id = 1; id <= testcases[i].maxFrame; ++id) {
            char buf[80];
            sprintf(buf, "/somagic/wnext/%s-%dp-case%d.png", testcases[i].name, testcases[i].playerId + 1, id);
            unique_ptr<AnalyzerResult> r = analyze(buf);

            EXPECT_EQ(testcases[i].rc, r->playerResult(testcases[i].playerId)->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS))
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
    // Next disappears here. After detecting next disappearing. we'd like to make userState playable.
    EXPECT_FALSE(rs[13]->playerResult(0)->userState.playable);
    EXPECT_TRUE(rs[14]->playerResult(0)->userState.playable);
    // Then controllable now.
    EXPECT_TRUE(rs[16]->playerResult(0)->userState.playable);
}

TEST_F(SomagicAnalyzerTest, vanishing)
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

    for (int i = 0; i <= 12; ++i) {
        EXPECT_TRUE(rs[i]->playerResult(0)->userState.playable) << i;
    }

    // vanishing should be detected here.
    // TODO(mayah): We might be able to detect in the previous frame?
    EXPECT_FALSE(rs[15]->playerResult(0)->userState.playable);
    // Since we've detected vanishing, "grounded" event should come here.
    EXPECT_TRUE(rs[15]->playerResult(0)->userState.grounded);
}

TEST_F(SomagicAnalyzerTest, ojamaDrop)
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

    for (int i = 0; i <= 28; ++i)
        EXPECT_TRUE(rs[i]->playerResult(1)->userState.playable);
    // Around here, analyzer can detect ojama puyo is dropped.
    EXPECT_FALSE(rs[30]->playerResult(1)->userState.playable);
    EXPECT_FALSE(rs[31]->playerResult(1)->userState.playable);
    // Then, next puyo will disappear.
    EXPECT_TRUE(rs[37]->playerResult(1)->userState.playable);
}

TEST_F(SomagicAnalyzerTest, gameStart)
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
        { 21, "BBPP  ", "BBPP  " },  // Because of fastDecision, we would be able to detect NEXT moving here.
        { 22, "BBPP  ", "BBPP  " },
        { 23, "BBPP  ", "BBPP  " },
        { 24, "BBPP  ", "BBPP  " },
        { 25, "BBPP  ", "BBPP  " },
        { 26, "BBPP  ", "BBPP  " },
        { 27, "BBPP  ", "BBPP  " },
        { 28, "BBPP  ", "BBPP  " },
        { 29, "BBPP  ", "BBPP  " },
        { 30, "BBPP  ", "BBPP  " },
        { 31, "BBPP--", "BBPP--" },  // Player 2, frame 31: Since some stars are located on NEXT2_AXIS,
        { 32, "BBPP--", "BBPP--" },  // analyzer might consider the axis color is YELLOW.
        { 33, "BBPPBB", "BBPP--" },  // However, we should fix in frame33 at least.
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
        { 46, "PPBB  ", "BBPPBB" },  // Player 1, frame 46: next1 will disappear. so next2 is moved to next1 on that time.
        { 47, "PPBB  ", "BBPPBB" },
        { 48, "PPBB  ", "BBPPBB" },
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

        if (c1[0] != '-') EXPECT_EQ(toRealColor(c1[0]), p1->adjustedField.realColor(NextPuyoPosition::CURRENT_AXIS)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[1] != '-') EXPECT_EQ(toRealColor(c1[1]), p1->adjustedField.realColor(NextPuyoPosition::CURRENT_CHILD)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[2] != '-') EXPECT_EQ(toRealColor(c1[2]), p1->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[3] != '-') EXPECT_EQ(toRealColor(c1[3]), p1->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[4] != '-') EXPECT_EQ(toRealColor(c1[4]), p1->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS)) << "Player1 : Frame " << expectations[i].frame;
        if (c1[5] != '-') EXPECT_EQ(toRealColor(c1[5]), p1->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD)) << "Player1 : Frame " << expectations[i].frame;

        if (c2[0] != '-') EXPECT_EQ(toRealColor(c2[0]), p2->adjustedField.realColor(NextPuyoPosition::CURRENT_AXIS)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[1] != '-') EXPECT_EQ(toRealColor(c2[1]), p2->adjustedField.realColor(NextPuyoPosition::CURRENT_CHILD)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[2] != '-') EXPECT_EQ(toRealColor(c2[2]), p2->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[3] != '-') EXPECT_EQ(toRealColor(c2[3]), p2->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[4] != '-') EXPECT_EQ(toRealColor(c2[4]), p2->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS)) << "Player2 : Frame " << expectations[i].frame;
        if (c2[5] != '-') EXPECT_EQ(toRealColor(c2[5]), p2->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD)) << "Player2 : Frame " << expectations[i].frame;
    }

    // ----------------------------------------------------------------------

    for (int i = 21; i <= 31; ++i) {
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS))
            << "Player 1: Frame" << i << ": NEXT1_AXIS";
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD))
            << "Player 1: Frame" << i << ": NEXT1_CHILD";
    }
    for (int i = 45; i <= 59; ++i) {
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS))
            << "Player 1: Frame" << i << ": NEXT1_AXIS";
        EXPECT_NE(RealColor::RC_EMPTY, rs[i]->playerResult(0)->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD))
            << "Player 1: Frame" << i << ": NEXT1_CHILD";
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
