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
        return RealColor::RC_EMPTY;

    if (hsv.s < 50 && 130 < hsv.v)
        return RealColor::RC_OJAMA;

    // The other colors are relatively easier. A bit tight range for now.
    if ((hsv.h <= 15 || 350 < hsv.h) && 70 < hsv.v)
        return RealColor::RC_RED;
    if (35 <= hsv.h && hsv.h <= 75 && 90 < hsv.v)
        return RealColor::RC_YELLOW;
    if (85 <= hsv.h && hsv.h <= 135 && 70 < hsv.v)
        return RealColor::RC_GREEN;
    // Detecting blue is relatively hard. Let's have a relaxed margin.
    if (180 <= hsv.h && hsv.h <= 255 && 60 < hsv.v)
        return RealColor::RC_BLUE;
    // Detecting purple is really hard. We'd like to have relaxed margin for purple.
    if (290 <= hsv.h && hsv.h < 340 && 65 < hsv.v)
        return RealColor::RC_PURPLE;

    // Hard to distinguish RED and PURPLE.
    if (340 <= hsv.h && hsv.h <= 350) {
        if (85 < hsv.v)
            return RealColor::RC_RED;
        if (65 < hsv.v)
            return RealColor::RC_PURPLE;
    }

    return RealColor::RC_EMPTY;
}

static RealColor estimateRealColorFromColorCount(int colorCount[NUM_REAL_COLORS], int threshold)
{
    static const RealColor colors[] = {
        RealColor::RC_RED,
        RealColor::RC_PURPLE,
        RealColor::RC_BLUE,
        RealColor::RC_YELLOW,
        RealColor::RC_GREEN
    };

    int maxCount = 0;
    RealColor result = RealColor::RC_EMPTY;

    for (int i = 0; i < 5; ++i) {
        int cnt = colorCount[static_cast<int>(colors[i])];
        if (cnt > threshold && cnt > maxCount) {
            result = colors[i];
            maxCount = cnt;
        }
    }

    // Since Ojama often makes their form smaller. So, it's a bit difficult to detect.
    // Let's relax a bit.
    int ojamaCount = colorCount[static_cast<int>(RealColor::RC_OJAMA)] * 3 / 2;
    if (ojamaCount > threshold && ojamaCount > maxCount)
        return RealColor::RC_OJAMA;

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
            Uint8 r, g, b;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            RGB rgb(r, g, b);
            HSV hsv = rgb.toHSV();

            RealColor rc = toRealColor(hsv);

            if (showsColor) {
                char buf[240];
                sprintf(buf, "%3d %3d : %3d %3d %3d : %7.3f %7.3f %7.3f : %s",
                        by, bx, static_cast<int>(r), static_cast<int>(g), static_cast<int>(b),
                        hsv.h, hsv.s, hsv.v, toString(rc));
                cout << buf << endl;
            }

            colorCount[0][static_cast<int>(rc)]++;
            colorCount[(by % 2) + 1][static_cast<int>(rc)]++;
        }
    }

    if (showsColor) {
        cout << "Color count:" << endl;
        for (int i = 0; i < NUM_REAL_COLORS; ++i) {
            RealColor rc = intToRealColor(i);
            cout << toString(rc) << " : " << colorCount[0][i] << endl;
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

    bool vanishing = rc[1] != rc[2] && (rc[1] == RealColor::RC_EMPTY || rc[2] == RealColor::RC_EMPTY);
    RealColor prc = vanishing ? (rc[1] != RealColor::RC_EMPTY ? rc[1] : rc[2]) : rc[0];

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

unique_ptr<DetectedField> SomagicAnalyzer::detectField(int pi,
                                                       const SDL_Surface* surface,
                                                       const SDL_Surface* prevSurface)
{
    unique_ptr<DetectedField> result(new DetectedField);

    // detect field
    for (int y = 1; y <= 12; ++y) {
        for (int x = 1; x <= 6; ++x) {
            Box b = BoundingBox::instance().get(pi, x, y);
            BoxAnalyzeResult r = analyzeBox(surface, b);

            result->field.set(x, y, r.realColor);
            result->vanishing.set(x, y, r.vanishing);
        }
    }

    // detect next
    {
        static const NextPuyoPosition np[4] = {
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
    }

    // detect next1 move
    {
        Box b = BoundingBox::instance().get(pi, NextPuyoPosition::NEXT1_AXIS);
        b = Box(b.sx, b.sy + b.h() / 2, b.dx, b.dy);
        BoxAnalyzeResult r = analyzeBox(surface, b);
        result->next1AxisMoving = (r.realColor == RealColor::RC_EMPTY);
    }

    // detect ojama
    {
        Box left = BoundingBox::instance().get(pi, 1, 0);
        Box right = BoundingBox::instance().get(pi, 6, 0);
        Box b = Box(left.sx, left.sy, right.dx, right.dy);
        result->setOjamaDropDetected(detectOjamaDrop(surface, prevSurface, b));
    }

    return result;
}

bool SomagicAnalyzer::detectOjamaDrop(const SDL_Surface* currentSurface,
                                      const SDL_Surface* prevSurface,
                                      const Box& box)
{
    // When prevSurface is NULL, we always think ojama is not dropped yet.
    if (!prevSurface)
        return false;

    int area = 0;
    double diffSum = 0;
    for (int by = box.sy; by <= box.dy; ++by) {
        for (int bx = box.sx; bx <= box.dx; ++bx) {
            Uint32 c1 = getpixel(currentSurface, bx, by);
            Uint8 r1, g1, b1;
            SDL_GetRGB(c1, currentSurface->format, &r1, &g1, &b1);

            // Since 3 SET MATCH etc. has RED or GREEN, we'd like to ignore them.
            RealColor rc = toRealColor(RGB(r1, g1, b1).toHSV());
            if (rc == RealColor::RC_RED || rc == RealColor::RC_GREEN)
                continue;

            Uint32 c2 = getpixel(prevSurface, bx, by);
            Uint8 r2, g2, b2;
            SDL_GetRGB(c2, prevSurface->format, &r2, &g2, &b2);

            double diff = sqrt((r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2));
            diffSum += diff;
            ++area;
        }
    }

    if (area == 0)
        return false;

    // Usually, (diffSum / area) is around 5. When ojama is dropped, it will be over 20.
    if (diffSum / area >= 17)
        return true;
    return false;
}

bool SomagicAnalyzer::isLevelSelect(const SDL_Surface* surface)
{
    Box b = BoundingBox::instance().getBy(BoundingBox::Region::LEVEL_SELECT);

    int whiteCount = 0;
    for (int bx = b.sx; bx <= b.dx; ++bx) {
        for (int by = b.sy; by <= b.dy; ++by) {
            Uint32 c = getpixel(surface, bx, by);
            Uint8 r, g, b;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            RGB rgb(r, g, b);
            HSV hsv = rgb.toHSV();
            RealColor rc = toRealColor(hsv);

            if (rc == RealColor::RC_OJAMA)
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
            Uint8 r, g, b;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            RGB rgb(r, g, b);
            HSV hsv = rgb.toHSV();
            RealColor rc = toRealColor(hsv);

            if (rc == RealColor::RC_OJAMA)
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
            Uint8 r, g, b;
            SDL_GetRGB(c, surface->format, &r, &g, &b);
            RGB rgb(r, g, b);
            HSV hsv = rgb.toHSV();

            RealColor rc = toRealColor(hsv);
            putpixel(surface, bx, by, toPixelColor(surface, rc));
        }
    }
}

// static
RealColor SomagicAnalyzer::estimateRealColor(const HSV& hsv)
{
    return toRealColor(hsv);
}
