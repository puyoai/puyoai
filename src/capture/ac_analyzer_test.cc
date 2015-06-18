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
        return analyzer.analyze(surf.get(), nullptr, nullptr, deque<unique_ptr<AnalyzerResult>>());
    }

    deque<unique_ptr<AnalyzerResult>> analyzeMultipleFrames(const vector<string>& imgFilenames,
                                                            bool userPlayable[2])
    {
        deque<unique_ptr<AnalyzerResult>> results;

        ACAnalyzer analyzer;
        bool firstResult = true;
        UniqueSDLSurface prev(emptyUniqueSDLSurface());
        UniqueSDLSurface prev2(emptyUniqueSDLSurface());

        for (const auto& imgFilename : imgFilenames) {
            string filename = FLAGS_testdata_dir + imgFilename;
            auto surface = makeUniqueSDLSurface(IMG_Load(filename.c_str()));
            CHECK(surface.get()) << "Failed to load "<< filename;
            auto r = analyzer.analyze(surface.get(), prev.get(), prev2.get(), results);
            if (firstResult) {
                r->mutablePlayerResult(0)->playable = userPlayable[0];
                r->mutablePlayerResult(1)->playable = userPlayable[1];
                firstResult = false;
            }
            results.push_front(move(r));
            prev2 = move(prev);
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
            EXPECT_EQ(field.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y))
                << "x=" << x << " y=" << y;
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
            EXPECT_EQ(field1.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y))
                << "x=" << x << " y=" << y;
            EXPECT_EQ(field2.color(x, y), r->playerResult(1)->adjustedField.realColor(x, y))
                << "x=" << x << " y=" << y;
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
            EXPECT_EQ(field.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y))
                << "x=" << x << " y=" << y;
        }
    }
}

TEST_F(ACAnalyzerTest, analyzeField4)
{
    unique_ptr<AnalyzerResult> r = analyze("/images/field/field4.png");

    EXPECT_EQ(CaptureGameState::PLAYING, r->state());

    const RealColorField field(
        ".P.G.."
        ".G.GR."
        "PRPGR."
        "RRRPG."
        "PPPGR.");

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            EXPECT_EQ(field.color(x, y), r->playerResult(0)->adjustedField.realColor(x, y))
                << "x=" << x << " y=" << y;
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

TEST_F(ACAnalyzerTest, wnextDetection)
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
                << " Player " << (testcases[i].playerId + 1)
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

TEST_F(ACAnalyzerTest, vanishing)
{
    vector<string> images;
    for (int i = 0; i < 32; ++i) {
        char buf[80];
        sprintf(buf, "/images/vanishing/frame%02d.png", i);
        images.push_back(buf);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    for (const auto& r : rs) {
        EXPECT_EQ(CaptureGameState::PLAYING, r->state());
    }

    for (int i = 0; i <= 24; ++i) {
        EXPECT_TRUE(rs[i]->playerResult(0)->playable) << i;
    }

    // vanishing should be detected here.
    EXPECT_TRUE(rs[29]->playerResult(0)->adjustedField.vanishing(2, 4));
    EXPECT_TRUE(rs[30]->playerResult(0)->adjustedField.vanishing(2, 4));
    EXPECT_TRUE(rs[31]->playerResult(0)->adjustedField.vanishing(2, 4));

    EXPECT_FALSE(rs[29]->playerResult(0)->playable);
    EXPECT_TRUE(rs[29]->playerResult(0)->userEvent.grounded);
}

TEST_F(ACAnalyzerTest, ojamaDrop)
{
    vector<string> images;
    for (int i = 0; i < 78; ++i) {
        char buf[80];
        sprintf(buf, "/images/ojama-drop/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    for (const auto& r : rs) {
        EXPECT_EQ(CaptureGameState::PLAYING, r->state());
    }

    for (int i = 0; i <= 56; ++i)
        EXPECT_TRUE(rs[i]->playerResult(1)->playable);
    // Around here, analyzer can detect ojama puyo is dropped.
    EXPECT_FALSE(rs[60]->playerResult(1)->playable);
    EXPECT_FALSE(rs[62]->playerResult(1)->playable);
    // Then, next puyo will disappear.
    EXPECT_TRUE(rs[72]->playerResult(1)->playable);
}

TEST_F(ACAnalyzerTest, nextArrival)
{
    vector<string> images;
    for (int i = 0; i < 50; ++i) {
        char buf[80];
        sprintf(buf, "/images/next-arrival/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    EXPECT_TRUE(rs[0]->playerResult(0)->playable);
    // Next disappears here. After detecting next disappearing. we'd like to make userEvent playable.
    EXPECT_FALSE(rs[26]->playerResult(0)->playable);
    EXPECT_TRUE(rs[26]->playerResult(0)->userEvent.decisionRequest);
    // Then controllable now.
    EXPECT_TRUE(rs[32]->playerResult(0)->playable);
}

TEST_F(ACAnalyzerTest, nextArrivalSousai)
{
    vector<string> images;
    for (int i = 0; i < 26; ++i) {
        char buf[80];
        sprintf(buf, "/images/next-arrival-sousai/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    EXPECT_TRUE(rs[18]->playerResult(1)->userEvent.decisionRequest);
    EXPECT_TRUE(rs[18]->playerResult(1)->userEvent.grounded);
}

TEST_F(ACAnalyzerTest, nextArrivalIrregular)
{
    vector<string> images;
    for (int i = 0; i < 92; ++i) {
        char buf[80];
        sprintf(buf, "/images/next-arrival-irregular/frame%02d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { true, true };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    EXPECT_TRUE(rs[78]->playerResult(1)->userEvent.decisionRequestAgain);
}

TEST_F(ACAnalyzerTest, gameStart)
{
    vector<string> images;
    for (int i = 0; i < 120; ++i) {
        char buf[80];
        sprintf(buf, "/images/game-start/frame%03d.png", i);
        string s = buf;
        images.push_back(s);
    }

    bool pgs[2] = { false, false };
    deque<unique_ptr<AnalyzerResult>> rs = analyzeMultipleFrames(images, pgs);

    // Frame 0-5 should be considered as LEVEL_SELECT.
    for (int i = 0; i < 6; ++i)
        EXPECT_EQ(CaptureGameState::LEVEL_SELECT, rs[i]->state());
    // At least frame 16, game state should be PLAYING.
    for (int i = 16; i < 120; ++i)
        EXPECT_EQ(CaptureGameState::PLAYING, rs[i]->state());

    const struct {
        int frameBegin;
        int frameEnd;
        const char* player1;
        const char* player2;
    } expectations[] = {
        // Frame 0-5 are level select. On level select, current puyo should be empty.
        // Also, we will use several frames to make NEXT stabilize.
        {   0,   6, "  ----", "  ----" },
        // On frame 18-,
        // Player2: Green character is on NEXT2, however, analyzer should say NEXT2 is still PP.
        {   6,  42, "  BBPP", "  BBPP" },
        // Because of animation, the decisionRequest might be sent in different frame. :-(
        {  42,  43, "  BBPP", "BBPP  " },
        // Because of fastDecision, we would be able to detect NEXT moving here.
        {  43,  60, "BBPP  ", "BBPP  " },
        // Player 2, frame 31: Since some stars are located on NEXT2_AXIS,
        // analyzer might consider the axis color is YELLOW.
        // However, we should fix in frame33 at least.
        {  60,  64, "BBPP--", "BBPP--" },
        {  64,  66, "BBPPBB", "BBPP--" },
        {  66,  95, "BBPPBB", "BBPPBB" },
        // Player 1, frame 95: next1 will disappear. so next2 is moved to next1 on that time.
        {  95, 118, "PPBB  ", "BBPPBB" },
        // Player 1, frame 120: Yellow will appeaer. But we will need 3 frames to detect it.
        { 118, 120, "PPBB--", "BBPPBB" },
    };

    for (const auto& expectation : expectations) {
        for (int i = expectation.frameBegin; i < expectation.frameEnd; ++i) {
            const PlayerAnalyzerResult* p1 = rs[i]->playerResult(0);
            const PlayerAnalyzerResult* p2 = rs[i]->playerResult(1);
            const char* c1 = expectation.player1;
            const char* c2 = expectation.player2;

            if (c1[0] != '-') EXPECT_EQ(toRealColor(c1[0]), p1->adjustedField.realColor(NextPuyoPosition::CURRENT_AXIS)) << "Player1 : Frame " << i;
            if (c1[1] != '-') EXPECT_EQ(toRealColor(c1[1]), p1->adjustedField.realColor(NextPuyoPosition::CURRENT_CHILD)) << "Player1 : Frame " << i;
            if (c1[2] != '-') EXPECT_EQ(toRealColor(c1[2]), p1->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS)) << "Player1 : Frame " << i;
            if (c1[3] != '-') EXPECT_EQ(toRealColor(c1[3]), p1->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD)) << "Player1 : Frame " << i;
            if (c1[4] != '-') EXPECT_EQ(toRealColor(c1[4]), p1->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS)) << "Player1 : Frame " << i;
            if (c1[5] != '-') EXPECT_EQ(toRealColor(c1[5]), p1->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD)) << "Player1 : Frame " << i;

            if (c2[0] != '-') EXPECT_EQ(toRealColor(c2[0]), p2->adjustedField.realColor(NextPuyoPosition::CURRENT_AXIS)) << "Player2 : Frame " << i;
            if (c2[1] != '-') EXPECT_EQ(toRealColor(c2[1]), p2->adjustedField.realColor(NextPuyoPosition::CURRENT_CHILD)) << "Player2 : Frame " << i;
            if (c2[2] != '-') EXPECT_EQ(toRealColor(c2[2]), p2->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS)) << "Player2 : Frame " << i;
            if (c2[3] != '-') EXPECT_EQ(toRealColor(c2[3]), p2->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD)) << "Player2 : Frame " << i;
            if (c2[4] != '-') EXPECT_EQ(toRealColor(c2[4]), p2->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS)) << "Player2 : Frame " << i;
            if (c2[5] != '-') EXPECT_EQ(toRealColor(c2[5]), p2->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD)) << "Player2 : Frame " << i;
        }
    }
}

TEST_F(ACAnalyzerTest, exhaustivePuyoDetection)
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
        ASSERT_TRUE(surf.get());

        for (int x = 0; (x + 1) * WIDTH <= surf->w; ++x) {
            for (int y = 0; (y + 1) * HEIGHT <= surf->h; ++y) {
                Box b(x * WIDTH, y * HEIGHT, (x + 1) * WIDTH, (y + 1) * HEIGHT);

                RealColor rc = analyzer.analyzeBoxWithRecognizer(surf.get(), b);
                EXPECT_EQ(color, rc) << "filename=" << filename << " x=" << x << " y=" << y;
            }
        }
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
