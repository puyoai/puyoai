#include "capture/ac_analyzer.h"

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

class ACAnalyzerTest : public testing::Test {
protected:
    unique_ptr<AnalyzerResult> analyze(const string& imgFilename)
    {
        string filename = FLAGS_testdata_dir + imgFilename;
        UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(filename.c_str())));
        CHECK(surf.get()) << "Failed to load " << filename;

        ACAnalyzer analyzer;
        return analyzer.analyze(surf.get(), nullptr, deque<unique_ptr<AnalyzerResult>>());
    }

    deque<unique_ptr<AnalyzerResult>> analyzeMultipleFrames(const vector<string>& imgFilenames,
                                                            bool userPlayable[2])
    {
        deque<unique_ptr<AnalyzerResult>> results;

        ACAnalyzer analyzer;
        bool firstResult = true;
        UniqueSDLSurface prev(emptyUniqueSDLSurface());

        for (const auto& imgFilename : imgFilenames) {
            string filename = FLAGS_testdata_dir + imgFilename;
            auto surface = makeUniqueSDLSurface(IMG_Load(filename.c_str()));
            CHECK(surface.get()) << "Failed to load "<< filename;
            auto r = analyzer.analyze(surface.get(), prev.get(), results);
            if (firstResult) {
                r->mutablePlayerResult(0)->playable = userPlayable[0];
                r->mutablePlayerResult(1)->playable = userPlayable[1];
                firstResult = false;
            }
            results.push_front(move(r));
            prev = move(surface);
        }

        reverse(results.begin(), results.end());
        return results;
    }
};

TEST_F(ACAnalyzerTest, estimateRealColor)
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
        { RealColor::RC_PURPLE, RGB( 90,  51,  58) },
        { RealColor::RC_PURPLE, RGB( 98,  40,  50) },
        { RealColor::RC_PURPLE, RGB( 94,  21,  78) },
        { RealColor::RC_PURPLE, RGB( 77,  23,  35) },
        { RealColor::RC_PURPLE, RGB( 84,  45,  52) },
        { RealColor::RC_PURPLE, RGB(135,  34, 142) },
    };
    int size = ARRAY_SIZE(testcases);

    for (int i = 0; i < size; ++i) {
        EXPECT_EQ(testcases[i].expected, ACAnalyzer::estimateRealColor(testcases[i].rgb.toHSV()))
            << " expected=" << toString(testcases[i].expected)
            << " actual=" << toString(ACAnalyzer::estimateRealColor(testcases[i].rgb.toHSV()))
            << " RGB=" << testcases[i].rgb.toString()
            << " HSV=" << testcases[i].rgb.toHSV().toString();
    }
}

TEST_F(ACAnalyzerTest, analyzeField1)
{
    unique_ptr<AnalyzerResult> r = analyze("/images/field/field1.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    const RealColorField field(
        ".P...."
        "GG...."
        "GY...."
        "YBY..."
        "YPP..."
        "BBBPB.");

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            EXPECT_EQ(field.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y));
        }
    }
}

TEST_F(ACAnalyzerTest, analyzeField2)
{
    unique_ptr<AnalyzerResult> r = analyze("/images/field/field2.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    const RealColorField field1(
        "...G.."  // 12
        "...R.."
        "...R.."
        "...G.."
        "...G.."  // 8
        "..GP.."
        "..YY.."
        "..YY.."
        "..GP.."  // 4
        "..GP.."
        "..GP.."
        "ORRR..");

    const RealColorField field2(
        "...P.."  // 12
        "O..Y.."
        "O..Y.."
        "OO.P.."
        "RO.P.."  // 8
        "RP.G.G"
        "YRRG.R"
        "PYOPGG"
        "YPRYRO"  // 4
        "PYYRRO"
        "PRPOOR"
        "PRGGYY");

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            EXPECT_EQ(field1.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y));
            EXPECT_EQ(field2.color(x, y), r->playerResult(1)->adjustedField.realColor(x, y));
        }
    }
}

TEST_F(ACAnalyzerTest, analyzeField3)
{
    unique_ptr<AnalyzerResult> r = analyze("/images/field/field3.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    const RealColorField field(
        "PO...."  // 12
        "BRPRBB"
        "PPGRGB"
        "BRBPRG"
        "RPBPBG"  // 8
        "BGROBG"
        "PGBOGB"
        "GPOGGR"
        "GRBBPP"  // 4
        "BRPPOO"
        "GRBRRP"
        "PBGPPB");

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            EXPECT_EQ(field.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y));
        }
    }
}

TEST_F(ACAnalyzerTest, analyzeFieldOjama)
{
    const RealColorField field(
        "...P.O"
        "OOYP.O"
        "OOOO.O"
        "OOOOBO"
        "OOOOOR"
        "YYOOOP"
        "BBOYOY"
        "BYBYOY");

    for (int caseNo = 1; caseNo <= 5; ++caseNo) {
        char filename[80];
        sprintf(filename, "/images/field/field-ojama%d.png", caseNo);
        unique_ptr<AnalyzerResult> r = analyze(filename);
        EXPECT_EQ(CaptureGameState::PLAYING, r->state());
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                EXPECT_EQ(field.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y))
                    << caseNo << ' ' << x << ' ' << y;
            }
        }

    }

}

TEST_F(ACAnalyzerTest, DISABLED_exhaustivePuyoDetection)
{
    const int WIDTH = 16;
    const int HEIGHT = 16;

    const pair<string, RealColor> testcases[] = {
        make_pair((FLAGS_testdata_dir + "/images/puyo/empty.png"), RealColor::RC_EMPTY),
        make_pair((FLAGS_testdata_dir + "/images/puyo/empty-blur.png"), RealColor::RC_EMPTY),

        make_pair((FLAGS_testdata_dir + "/images/puyo/ojama.png"), RealColor::RC_OJAMA),
        make_pair((FLAGS_testdata_dir + "/images/puyo/ojama-blur.png"), RealColor::RC_OJAMA),

        make_pair((FLAGS_testdata_dir + "/images/puyo/red.png"), RealColor::RC_RED),
        make_pair((FLAGS_testdata_dir + "/images/puyo/red-blur.png"), RealColor::RC_RED),

        make_pair((FLAGS_testdata_dir + "/images/puyo/blue.png"), RealColor::RC_BLUE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/blue-blur.png"), RealColor::RC_BLUE),

        make_pair((FLAGS_testdata_dir + "/images/puyo/yellow.png"), RealColor::RC_YELLOW),
        make_pair((FLAGS_testdata_dir + "/images/puyo/yellow-blur.png"), RealColor::RC_YELLOW),

        make_pair((FLAGS_testdata_dir + "/images/puyo/green.png"), RealColor::RC_GREEN),
        make_pair((FLAGS_testdata_dir + "/images/puyo/green-blur.png"), RealColor::RC_GREEN),

        make_pair((FLAGS_testdata_dir + "/images/puyo/purple.png"), RealColor::RC_PURPLE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/purple-blur.png"), RealColor::RC_PURPLE),
    };

    ACAnalyzer analyzer;
    for (const auto& testcase : testcases) {
        const string& filename = testcase.first;
        const RealColor color = testcase.second;

        UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(filename.c_str())));

        for (int x = 0; (x + 1) * WIDTH <= surf->w; ++x) {
            for (int y = 0; (y + 1) * HEIGHT <= surf->h; ++y) {
                Box b(x * WIDTH, y * HEIGHT, (x + 1) * WIDTH, (y + 1) * HEIGHT);

                BoxAnalyzeResult result = analyzer.analyzeBox(surf.get(), b);
                EXPECT_EQ(color, result.realColor) << "filename=" << filename << " x=" << x << " y=" << y;
            }
        }
    }
}

TEST_F(ACAnalyzerTest, DISABLED_wnextDetection)
{
    struct Testcase {
        int playerId;
        RealColor rc;
        const char* name;
        int maxFrame;
    };

    vector<Testcase> testcases = {
        { 0, RealColor::RC_BLUE,   "blue",   16 },
        { 0, RealColor::RC_GREEN,  "green",  16 },
        { 0, RealColor::RC_PURPLE, "purple", 12 },
        { 0, RealColor::RC_RED,    "red",    16 },
        { 0, RealColor::RC_YELLOW, "yellow", 14 },
        { 1, RealColor::RC_BLUE,   "blue",   12 },
        { 1, RealColor::RC_GREEN,  "green",  16 },
        { 1, RealColor::RC_PURPLE, "purple", 16 },
        { 1, RealColor::RC_RED,    "red",    18 },
        { 1, RealColor::RC_YELLOW, "yellow", 16 },
    };

    for (size_t i = 0; i < testcases.size(); ++i) {
        for (int id = 1; id <= testcases[i].maxFrame; ++id) {
            char buf[80];
            sprintf(buf, "/images/wnext/%s-%dp-case%02d.png", testcases[i].name, testcases[i].playerId + 1, id);
            unique_ptr<AnalyzerResult> r = analyze(buf);

            EXPECT_EQ(testcases[i].rc, r->playerResult(testcases[i].playerId)->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS))
                << " Player " << testcases[i].playerId
                << " Testcase " << testcases[i].name
                << " Case " << id;
        }
    }
}

TEST_F(ACAnalyzerTest, levelSelect)
{
    const char* testcases[] = {
        "/images/level-select/level-select1.png",
        "/images/level-select/level-select2.png",
    };

    for (const auto& testcase : testcases) {
        unique_ptr<AnalyzerResult> r = analyze(testcase);
        EXPECT_EQ(CaptureGameState::LEVEL_SELECT, r->state());
    }
}

TEST_F(ACAnalyzerTest, gameFinished)
{
    struct TestCase {
        const char* filename;
        CaptureGameState expectedState;
    };
    const TestCase testcases[] = {
        { "/images/game-finished/game-finished1.png", CaptureGameState::GAME_FINISHED_WITH_1P_WIN },
        { "/images/game-finished/game-finished2.png", CaptureGameState::MATCH_FINISHED_WITH_1P_WIN },
        { "/images/game-finished/game-finished3.png", CaptureGameState::GAME_FINISHED_WITH_2P_WIN },
        { "/images/game-finished/game-finished4.png", CaptureGameState::MATCH_FINISHED_WITH_1P_WIN }
    };

    for (const auto& testcase : testcases) {
        unique_ptr<AnalyzerResult> r = analyze(testcase.filename);
        EXPECT_EQ(testcase.expectedState, r->state());
    }
}

#if 0

TEST_F(ACAnalyzerTest, nextArrival)
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

    EXPECT_TRUE(rs[0]->playerResult(0)->playable);
    // Next disappears here. After detecting next disappearing. we'd like to make userEvent playable.
    EXPECT_FALSE(rs[13]->playerResult(0)->playable);
    EXPECT_TRUE(rs[13]->playerResult(0)->userEvent.decisionRequest);
    EXPECT_TRUE(rs[14]->playerResult(0)->playable);
    // Then controllable now.
    EXPECT_TRUE(rs[16]->playerResult(0)->playable);
}

TEST_F(ACAnalyzerTest, nextArrivalSousai)
{
    vector<string> images;
    for (int i = 0; i <= 12; ++i) {
        char buf[80];
        sprintf(buf, "/somagic/next-arrival-sousai/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    EXPECT_TRUE(rs[9]->playerResult(1)->userEvent.decisionRequest || rs[10]->playerResult(1)->userEvent.decisionRequest);
    EXPECT_TRUE(rs[9]->playerResult(1)->userEvent.grounded || rs[10]->playerResult(1)->userEvent.grounded);
}

TEST_F(ACAnalyzerTest, irregularNextArrival)
{
    vector<string> images;
    for (int i = 0; i <= 45; ++i) {
        char buf[80];
        sprintf(buf, "/somagic/next-arrival-irregular/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    EXPECT_TRUE(rs[38]->playerResult(1)->userEvent.decisionRequestAgain || rs[39]->playerResult(1)->userEvent.decisionRequestAgain || rs[40]->playerResult(1)->userEvent.decisionRequestAgain);
}

TEST_F(ACAnalyzerTest, vanishing)
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
        EXPECT_TRUE(rs[i]->playerResult(0)->playable) << i;
    }

    // vanishing should be detected here.
    // TODO(mayah): We might be able to detect in the previous frame?
    EXPECT_FALSE(rs[15]->playerResult(0)->playable);
    // Since we've detected vanishing, "grounded" event should come here.
    EXPECT_TRUE(rs[15]->playerResult(0)->userEvent.grounded);
}

TEST_F(ACAnalyzerTest, nonfastmove)
{
    vector<string> images;
    for (int i = 0; i <= 33; ++i) {
        char buf[80];
        sprintf(buf, "/somagic/nonfastmove/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    for (const auto& r : rs) {
        EXPECT_EQ(CaptureGameState::PLAYING, r->state());
    }

    // Since this is not fast move, we should not request decision in
    // frame26 or frame27.
    EXPECT_FALSE(rs[25]->playerResult(1)->userEvent.decisionRequest);
    EXPECT_FALSE(rs[26]->playerResult(1)->userEvent.decisionRequest);
    EXPECT_FALSE(rs[27]->playerResult(1)->userEvent.decisionRequest);
}

TEST_F(ACAnalyzerTest, ojamaDrop)
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
        EXPECT_TRUE(rs[i]->playerResult(1)->playable);
    // Around here, analyzer can detect ojama puyo is dropped.
    EXPECT_FALSE(rs[30]->playerResult(1)->playable);
    EXPECT_FALSE(rs[31]->playerResult(1)->playable);
    // Then, next puyo will disappear.
    EXPECT_TRUE(rs[37]->playerResult(1)->playable);
}

TEST_F(ACAnalyzerTest, gameStart)
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
        { 46, "BBPPBB", "BBPPBB" },
        { 47, "PPBB  ", "BBPPBB" },  // Player 1, frame 47: next1 will disappear. so next2 is moved to next1 on that time.
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
#endif

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
