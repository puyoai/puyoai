#include "capture/somagic_analyzer.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "capture/color.h"
#include "gui/pixel_color.h"
#include "gui/util.h"

using namespace std;

static RealColor toRealColor(const HSV& hsv)
{
    if (hsv.v < 38)
        return RC_EMPTY;

    if (130 < hsv.v && hsv.s < 30)
        return RC_OJAMA;

    // Detecting purple is really hard. We'd like to have relaxed margin for purple.
    if (13 <= hsv.h && hsv.h < 75 && 65 < hsv.v)
        return RC_PURPLE;

    // Detecting blue is relatively hard. Let's have a relaxed margin.
    if (130 < hsv.h && hsv.h < 190 && 60 < hsv.v)
        return RC_BLUE;

    // The other colors are relatively easier. A bit tight range for now.
    if ((hsv.h < 10 || 340 < hsv.h) && 70 < hsv.v)
        return RC_RED;
    if (240 < hsv.h && hsv.h < 270 && 90 < hsv.v)
        return RC_GREEN;
    if (310 < hsv.h && hsv.h < 320 && 95 < hsv.v)
        return RC_YELLOW;

    return RC_EMPTY;
}

static RealColor estimateRealColorFromColorCount(int colorCount[NUM_REAL_COLORS], int threshold)
{
    static const RealColor colors[] = { RC_RED, RC_PURPLE, RC_BLUE, RC_YELLOW, RC_GREEN };

    int cnt = 0;
    RealColor result = RC_EMPTY;

    for (int i = 0; i < 5; ++i) {
        if (colorCount[colors[i]] > threshold && colorCount[colors[i]] > cnt) {
            result = colors[i];
            cnt = colorCount[colors[i]];
        }
    }

    // Since Ojama often makes their form smaller. So, it's a bit difficult to detect.
    // Let's relax a bit.
    if (colorCount[RC_OJAMA] > threshold * 2 / 3 && colorCount[RC_OJAMA] > cnt * 2 / 3) {
        return RC_OJAMA;
    }

    return result;
}

SomagicAnalyzer::SomagicAnalyzer()
{
    // TODO(mayah): initializing here seems wrong.
    BoundingBox::instance().setGenerator(68, 80, 32, 32);
    BoundingBox::instance().setRegion(BoundingBox::Region::LEVEL_SELECT, Box(260, 256, 270, 280));
    BoundingBox::instance().setRegion(BoundingBox::Region::GAME_FINISHED, Box(292, 352, 420, 367));
}

SomagicAnalyzer::~SomagicAnalyzer()
{
}

BoxAnalyzeResult SomagicAnalyzer::analyzeBox(const SDL_Surface* surface, const Box& b, bool showsColor) const
{
    int colorCount[3][NUM_REAL_COLORS] = { { 0 } };

    // We'd like to take padding 1 pixel.
    for (int by = b.sy + 1; by <= b.dy - 1; ++by) {
        for (int bx = b.sx + 1; bx <= b.dx - 1; ++bx) {
            Uint32 c = getpixel(surface, bx, by);
            Uint8 r, b, g;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            RGB rgb(r, b, g);
            HSV hsv = rgb.toHSV();

            RealColor rc = toRealColor(hsv);

            if (showsColor) {
                char buf[240];
                sprintf(buf, "%3d %3d : %3d %3d %3d : %7.3f %7.3f %7.3f : %d",
                        by, bx, static_cast<int>(r), static_cast<int>(g), static_cast<int>(b),
                        hsv.h, hsv.s, hsv.v, static_cast<int>(rc));
                cout << buf << endl;
            }

            colorCount[0][rc]++;
            colorCount[(by % 2) + 1][rc]++;
        }
    }

    // TODO(mayah): This is a bit cryptic.
    // whole puyo will be 32 x 32 (or 31x31?, anyway 31x31 > 32x30)
    // WNEXT2 will have smaller area.
    int area = b.w() * b.h();
    int threshold = (area >= 30 * 32) ? 50 : 20;
    int halfThreshold = (area >= 30 * 32) ? 25 : 15;

    RealColor rc[3] = {
        estimateRealColorFromColorCount(colorCount[0], threshold),
        estimateRealColorFromColorCount(colorCount[1], halfThreshold),
        estimateRealColorFromColorCount(colorCount[2], halfThreshold),
    };

    bool vanishing = rc[1] != rc[2] && (rc[1] == RC_EMPTY || rc[2] == RC_EMPTY);
    RealColor prc = vanishing ? (rc[1] != RC_EMPTY ? rc[1] : rc[2]) : rc[0];

    return BoxAnalyzeResult(prc, vanishing);
}

CaptureGameState SomagicAnalyzer::detectGameState(const SDL_Surface* surface)
{
    if (isLevelSelect(surface))
        return CaptureGameState::LEVEL_SELECT;

    if (isGameFinished(surface))
        return CaptureGameState::FINISHED;

    return CaptureGameState::PLAYING;
}

unique_ptr<DetectedField> SomagicAnalyzer::detectField(int pi, const SDL_Surface* surface)
{
    unique_ptr<DetectedField> result(new DetectedField);

    // detect field
    for (int y = 1; y <= 12; ++y) {
        for (int x = 1; x <= 6; ++x) {
            Box b = BoundingBox::instance().get(pi, x, y);
            BoxAnalyzeResult r = analyzeBox(surface, b);

            result->puyos[x-1][y-1] = r.realColor;
            result->vanishing[x-1][y-1] = r.vanishing;
        }
    }

    // detect next
    NextPuyoPosition np[4] = {
        NextPuyoPosition::NEXT1_AXIS,
        NextPuyoPosition::NEXT1_CHILD,
        NextPuyoPosition::NEXT2_AXIS,
        NextPuyoPosition::NEXT2_CHILD,
    };

    for (int i = 0; i < 4; ++i) {
        Box b = BoundingBox::instance().get(pi, np[i]);
        BoxAnalyzeResult r = analyzeBox(surface, b);
        result->setRealColor(np[i], r.realColor);
    }

    return result;
}

bool SomagicAnalyzer::isLevelSelect(const SDL_Surface* surface)
{
    Box b = BoundingBox::instance().getBy(BoundingBox::Region::LEVEL_SELECT);

    int whiteCount = 0;
    for (int bx = b.sx; bx <= b.dx; ++bx) {
        for (int by = b.sy; by <= b.dy; ++by) {
            Uint32 c = getpixel(surface, bx, by);
            Uint8 r, b, g;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            RGB rgb(r, b, g);
            HSV hsv = rgb.toHSV();
            RealColor rc = toRealColor(hsv);

            if (rc == RC_OJAMA)
                ++whiteCount;
        }
    }

    return whiteCount >= 40;
}

bool SomagicAnalyzer::isGameFinished(const SDL_Surface* surface)
{
    Box b = BoundingBox::instance().getBy(BoundingBox::Region::GAME_FINISHED);

    int whiteCount = 0;
    for (int bx = b.sx; bx <= b.dx; ++bx) {
        for (int by = b.sy; by <= b.dy; ++by) {
            Uint32 c = getpixel(surface, bx, by);
            Uint8 r, b, g;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            RGB rgb(r, b, g);
            HSV hsv = rgb.toHSV();
            RealColor rc = toRealColor(hsv);

            if (rc == RC_OJAMA)
                ++whiteCount;
        }
    }

    return whiteCount >= 50;
}

void SomagicAnalyzer::drawWithAnalysisResult(SDL_Surface* surface)
{
    for (int pi = 0; pi < 2; ++pi) {
        for (int y = 1; y <= 12; ++y) {
            for (int x = 1; x <= 6; ++x) {
                Box box = BoundingBox::instance().get(pi, x, y);
                drawBoxWithAnalysisResult(surface, box);
            }
        }
    }

    for (int pi = 0; pi < 2; ++pi) {
        Box bs[4] = {
            BoundingBox::instance().get(pi, NextPuyoPosition::NEXT1_AXIS),
            BoundingBox::instance().get(pi, NextPuyoPosition::NEXT1_CHILD),
            BoundingBox::instance().get(pi, NextPuyoPosition::NEXT2_AXIS),
            BoundingBox::instance().get(pi, NextPuyoPosition::NEXT2_CHILD)
        };

        for (int i = 0; i < 4; ++i) {
            drawBoxWithAnalysisResult(surface, bs[i]);
        }
    }

    drawBoxWithAnalysisResult(surface, BoundingBox::instance().getBy(BoundingBox::Region::LEVEL_SELECT));
    drawBoxWithAnalysisResult(surface, BoundingBox::instance().getBy(BoundingBox::Region::GAME_FINISHED));
}

void SomagicAnalyzer::drawBoxWithAnalysisResult(SDL_Surface* surface, const Box& box)
{
    for (int by = box.sy; by <= box.dy; ++by) {
        for (int bx = box.sx; bx <= box.dx; ++bx) {
            Uint32 c = getpixel(surface, bx, by);
            Uint8 r, b, g;
            SDL_GetRGB(c, surface->format, &r, &g, &b);
            RGB rgb(r, b, g);
            HSV hsv = rgb.toHSV();

            RealColor rc = toRealColor(hsv);
            putpixel(surface, bx, by, toPixelColor(surface, rc));
        }
    }
}
