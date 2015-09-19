#include "capture/ac_analyzer.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "capture/color.h"
#include "gui/pixel_color.h"
#include "gui/util.h"

using namespace std;

DEFINE_string(recognition_dir, RECOGNITION_DIR, "the directory path to recognition dir.");
DEFINE_bool(strict_ojama_recognition, true, "use strict ojama recognition.");

namespace {
const int BOX_THRESHOLD = 15;
const int SMALLER_BOX_THRESHOLD = 7;
}

static RealColor toRealColor(const HSV& hsv)
{
    if (hsv.v < 38)
        return RealColor::RC_EMPTY;

    if (hsv.s < 50 && 120 < hsv.v)
        return RealColor::RC_OJAMA;

    if (hsv.s < 10)
        return RealColor::RC_EMPTY;

    // The other colors are relatively easier. A bit tight range for now.
    if (hsv.h <= 15 && 70 < hsv.v)
        return RealColor::RC_RED;
    if (35 <= hsv.h && hsv.h <= 75 && 90 < hsv.v)
        return RealColor::RC_YELLOW;
    if (85 <= hsv.h && hsv.h <= 135 && 70 < hsv.v)
        return RealColor::RC_GREEN;
    // Detecting blue is relatively hard. Let's have a relaxed margin.
    if (160 <= hsv.h && hsv.h <= 255 && 50 < hsv.v)
        return RealColor::RC_BLUE;
    // Detecting purple is really hard. We'd like to have relaxed margin for purple.
    if (280 <= hsv.h && hsv.h < 340 && 50 < hsv.v)
        return RealColor::RC_PURPLE;

    // Hard to distinguish RED and PURPLE.
    if (340 <= hsv.h && hsv.h <= 360) {
        if (160 < hsv.s + hsv.v)
            return RealColor::RC_RED;
        if (50 < hsv.v)
            return RealColor::RC_PURPLE;
    }

    return RealColor::RC_EMPTY;
}

static RealColor estimateRealColorFromColorCount(int colorCount[NUM_REAL_COLORS],
                                                 int threshold,
                                                 ACAnalyzer::AllowOjama allowOjama = ACAnalyzer::AllowOjama::ALLOW_OJAMA,
                                                 ACAnalyzer::AnalyzeBoxFunc analyzeBoxFunc = ACAnalyzer::AnalyzeBoxFunc::NORMAL)
{
    static const RealColor colors[] = {
        RealColor::RC_RED,
        RealColor::RC_BLUE,
        RealColor::RC_GREEN,
        RealColor::RC_PURPLE,
        RealColor::RC_YELLOW, // YELLOW must be the last.
    };

    int maxCount = 0;
    RealColor result = RealColor::RC_EMPTY;

    if (analyzeBoxFunc == ACAnalyzer::AnalyzeBoxFunc::NEXT2) {
        for (int i = 0; i < 5; ++i) {
            int cnt = colorCount[ordinal(colors[i])];
            if (cnt < threshold)
                continue;

            if (colors[i] != RealColor::RC_YELLOW) {
                if (cnt > maxCount) {
                    maxCount = cnt;
                    result = colors[i];
                }
            } else {
                if (maxCount == 0) {
                    maxCount = cnt;
                    result = colors[i];
                }
            }
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            int cnt = colorCount[static_cast<int>(colors[i])];
            if (colors[i] == RealColor::RC_YELLOW)
                cnt = cnt * 4 / 5;
            if (cnt >= threshold && cnt > maxCount) {
                result = colors[i];
                maxCount = cnt;
            }
        }
    }

    // Since Ojama often makes their form smaller. So, it's a bit difficult to detect.
    // Let's relax a bit.
    if (allowOjama == ACAnalyzer::AllowOjama::ALLOW_OJAMA) {
        int ojamaCount = colorCount[static_cast<int>(RealColor::RC_OJAMA)];
        if (ojamaCount * 3 / 2 > threshold) {
            // ojama sometimes looks green.
            if (result == RealColor::RC_GREEN && ojamaCount * 3 / 2 > maxCount) {
                return RealColor::RC_OJAMA;
            }

            if (ojamaCount > maxCount) {
                return RealColor::RC_OJAMA;
            }
        }
    }

    return result;
}

ACAnalyzer::ACAnalyzer() :
    recognizer_(FLAGS_recognition_dir)
{
}

ACAnalyzer::~ACAnalyzer()
{
}

RealColor ACAnalyzer::analyzeBox(const SDL_Surface* surface, const Box& b,
                                 AllowOjama allowOjama,
                                 ShowDebugMessage showsColor,
                                 AnalyzeBoxFunc analyzeBoxFunc) const
{
    int colorCount[NUM_REAL_COLORS] {};

    for (int by = b.sy; by < b.dy; ++by) {
        for (int bx = b.sx; bx < b.dx; ++bx) {
            Uint32 c = getpixel(surface, bx, by);
            Uint8 r, g, b;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            RGB rgb(r, g, b);
            HSV hsv = rgb.toHSV();

            RealColor rc = toRealColor(hsv);

            if (showsColor == ShowDebugMessage::SHOW_DEBUG_MESSAGE) {
                // TODO(mayah): stringstream?
                char buf[240];
                sprintf(buf, "%3d %3d : %3d %3d %3d : %7.3f %7.3f %7.3f : %s",
                        by, bx, static_cast<int>(r), static_cast<int>(g), static_cast<int>(b),
                        hsv.h, hsv.s, hsv.v, toString(rc).c_str());
                cout << buf << endl;
            }

            colorCount[static_cast<int>(rc)]++;
        }
    }

    if (showsColor == ShowDebugMessage::SHOW_DEBUG_MESSAGE) {
        cout << "Color count:" << endl;
        for (int i = 0; i < NUM_REAL_COLORS; ++i) {
            RealColor rc = intToRealColor(i);
            cout << toString(rc) << " : " << colorCount[i] << endl;
        }
    }

    // TODO(mayah): This is a bit cryptic.
    // whole puyo will be 16 x 16 (or 15x15?, anyway 15x15 > 16x14)
    // WNEXT2 will have smaller area.
    int area = b.w() * b.h();
    int threshold = (area >= 16 * 14) ? BOX_THRESHOLD : SMALLER_BOX_THRESHOLD;

    return estimateRealColorFromColorCount(colorCount, threshold, allowOjama, analyzeBoxFunc);
}

RealColor ACAnalyzer::analyzeBoxWithRecognizer(const SDL_Surface* surface, const Box& b) const
{
    CHECK_EQ(16, b.dx - b.sx);
    CHECK_EQ(16, b.dy - b.sy);

    int pos = 0;
    double features[16 * 16 * 3];
    for (int by = b.sy; by < b.dy; ++by) {
        for (int bx = b.sx; bx < b.dx; ++bx) {
            Uint32 c = getpixel(surface, bx, by);
            Uint8 r, g, b;
            SDL_GetRGB(c, surface->format, &r, &g, &b);

            features[pos++] = r;
            features[pos++] = g;
            features[pos++] = b;
        }
    }
    CHECK_EQ(16 * 16 * 3, pos);
    return recognizer_.recognize(features);
}

RealColor ACAnalyzer::analyzeBoxInField(const SDL_Surface* surface, const Box& b) const
{
    RealColor rc = analyzeBox(surface, b);
    switch (rc) {
    case RealColor::RC_GREEN: {
        RealColor rc2 = analyzeBoxWithRecognizer(surface, b);
        if (rc2 == RealColor::RC_EMPTY)
            rc = rc2;
        break;
    }
    case RealColor::RC_YELLOW: {
        RealColor rc2 = analyzeBoxWithRecognizer(surface, b);
        if (rc2 == RealColor::RC_EMPTY || rc2 == RealColor::RC_PURPLE || rc2 == RealColor::RC_OJAMA)
            rc = rc2;
        break;
    }
    case RealColor::RC_OJAMA: {
        RealColor rc2 = analyzeBoxWithRecognizer(surface, b);
        if (rc2 == RealColor::RC_PURPLE)
            rc = rc2;
        break;
    }
    default:
        break;
    }

    return rc;
}

RealColor ACAnalyzer::analyzeBoxNext2(const SDL_Surface* surface, const Box& b) const
{
    return analyzeBox(surface, b, AllowOjama::DONT_ALLOW_OJAMA, ShowDebugMessage::DONT_SHOW_DEBUG, AnalyzeBoxFunc::NEXT2);
}

CaptureGameState ACAnalyzer::detectGameState(const SDL_Surface* surface)
{
    if (isLevelSelect(surface))
        return CaptureGameState::LEVEL_SELECT;

    if (isGameFinished(surface)) {
        bool matchEnd = isMatchEnd(surface);
        bool p1Dead = isDead(0, surface);
        bool p2Dead = isDead(1, surface);
        if (matchEnd) {
            if (p1Dead && p2Dead)
                return CaptureGameState::MATCH_FINISHED_WITH_DRAW;
            if (p2Dead)
                return CaptureGameState::MATCH_FINISHED_WITH_1P_WIN;
            if (p1Dead)
                return CaptureGameState::MATCH_FINISHED_WITH_2P_WIN;
        } else {
            if (p1Dead && p2Dead)
                return CaptureGameState::GAME_FINISHED_WITH_DRAW;
            if (p2Dead)
                return CaptureGameState::GAME_FINISHED_WITH_1P_WIN;
            if (p1Dead)
                return CaptureGameState::GAME_FINISHED_WITH_2P_WIN;
        }

        LOG(ERROR) << "game finished, but no one is lost?";
        return CaptureGameState::GAME_FINISHED_WITH_DRAW;
    }

    return CaptureGameState::PLAYING;
}

unique_ptr<DetectedField> ACAnalyzer::detectField(int pi,
                                                  const SDL_Surface* surface,
                                                  const SDL_Surface* prev2Surface,
                                                  const SDL_Surface* prev3Surface)
{
    unique_ptr<DetectedField> result(new DetectedField);

    // detect field
    for (int y = 1; y <= 12; ++y) {
        for (int x = 1; x <= 6; ++x) {
            Box b = BoundingBox::boxForAnalysis(pi, x, y);
            RealColor rc = analyzeBoxInField(surface, b);
            result->field.set(x, y, rc);
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
            Box b = BoundingBox::boxForAnalysis(pi, np[i]);
            RealColor rc = analyzeBox(surface, b, AllowOjama::DONT_ALLOW_OJAMA);
            result->setRealColor(np[i], rc);
        }
    }

    // detect next1 move
    {
        Box b = BoundingBox::boxForAnalysis(pi, NextPuyoPosition::NEXT1_AXIS);
        b = Box(b.sx, b.sy + b.h() / 2, b.dx, b.dy);
        RealColor rc = analyzeBox(surface, b, AllowOjama::DONT_ALLOW_OJAMA);
        result->next1AxisMoving = (rc == RealColor::RC_EMPTY);
    }

    // detect ojama. Comparing with prev2 and prev3.
    // We don't use prev1, because if the field is gradually changed, we cannot detect it.
    {
        Box left = BoundingBox::boxForAnalysis(pi, 1, 0);
        Box right = BoundingBox::boxForAnalysis(pi, 6, 0);
        Box b = Box(left.sx, left.sy, right.dx, right.dy);

        bool detected = false;
        if (FLAGS_strict_ojama_recognition) {
            detected = detectOjamaDrop(surface, prev2Surface, b) && detectOjamaDrop(surface, prev3Surface, b);
        } else {
            detected = detectOjamaDrop(surface, prev2Surface, b);
        }


        result->setOjamaDropDetected(detected);
    }

    return result;
}

bool ACAnalyzer::detectOjamaDrop(const SDL_Surface* currentSurface,
                                 const SDL_Surface* prev2Surface,
                                 const Box& box)
{
    // When prevSurface is NULL, we always think ojama is not dropped yet.
    if (!prev2Surface)
        return false;

    int area = 0;
    double diffSum = 0;
    for (int by = box.sy; by < box.dy; ++by) {
        for (int bx = box.sx; bx < box.dx; ++bx) {
            Uint32 c1 = getpixel(currentSurface, bx, by);
            Uint8 r1, g1, b1;
            SDL_GetRGB(c1, currentSurface->format, &r1, &g1, &b1);

            // Since 3 SET MATCH etc. has RED or GREEN, we'd like to ignore them.
            RealColor rc = toRealColor(RGB(r1, g1, b1).toHSV());
            if (rc == RealColor::RC_RED || rc == RealColor::RC_GREEN)
                continue;

            Uint32 c2 = getpixel(prev2Surface, bx, by);
            Uint8 r2, g2, b2;
            SDL_GetRGB(c2, prev2Surface->format, &r2, &g2, &b2);

            double diff = sqrt((r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2));
            diffSum += diff;
            ++area;
        }
    }

    if (area == 0)
        return false;

#if 0
    cout << "diffSum=" << diffSum << " area=" << area << " ratio=" << (diffSum / area) << endl;
#endif

    // Usually, (diffSum / area) is around 20. When ojama is dropped, it will be over 50.
    if (diffSum / area >= 30)
        return true;
    return false;
}

bool ACAnalyzer::isLevelSelect(const SDL_Surface* surface)
{
    const Box boxes[] {
        BoundingBox::boxForAnalysis(BoundingBox::Region::LEVEL_SELECT_1P),
        BoundingBox::boxForAnalysis(BoundingBox::Region::LEVEL_SELECT_2P),
    };

    for (const Box& b : boxes) {
        int whiteCount = 0;
        for (int bx = b.sx; bx < b.dx; ++bx) {
            for (int by = b.sy; by < b.dy; ++by) {
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

        if (whiteCount >= 20)
            return true;
    }

    return false;
}

bool ACAnalyzer::isGameFinished(const SDL_Surface* surface)
{
    Box b = BoundingBox::boxForAnalysis(BoundingBox::Region::GAME_FINISHED);

    int whiteCount = 0;
    for (int bx = b.sx; bx < b.dx; ++bx) {
        for (int by = b.sy; by < b.dy; ++by) {
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

bool ACAnalyzer::isDead(int playerId, const SDL_Surface* surface)
{
    // Since (3, 0)-(6, 0) of player2 field might contain 'FREE PLAY' string.
    // So, we check only (1, 0) and (2, 0).
    RealColor rc1 = analyzeBox(surface, BoundingBox::boxForAnalysis(playerId, 1, 0));
    RealColor rc2 = analyzeBox(surface, BoundingBox::boxForAnalysis(playerId, 2, 0));

    return rc1 != RealColor::RC_YELLOW && rc2 != RealColor::RC_YELLOW;
}

bool ACAnalyzer::isMatchEnd(const SDL_Surface* surface)
{
    Box b1 = BoundingBox::boxForAnalysis(0, 7, 2);
    Box b2 = BoundingBox::boxForAnalysis(0, 12, 0);

    int red = 0;
    int blue = 0;

    for (int x = b1.dx; x < b2.dx; ++x) {
        for (int y = b1.dy; y < b2.dy; ++y) {
            Uint32 c = getpixel(surface, x, y);
            Uint8 r, g, b;
            SDL_GetRGB(c, surface->format, &r, &g, &b);
            RGB rgb(r, g, b);
            HSV hsv = rgb.toHSV();

            RealColor rc = toRealColor(hsv);
            if (rc == RealColor::RC_RED)
                ++red;
            if (rc == RealColor::RC_BLUE)
                ++blue;
        }
    }

    if (red > 100 && blue > 100)
        return true;

    return false;
}

void ACAnalyzer::drawWithAnalysisResult(SDL_Surface* surface)
{
    for (int pi = 0; pi < 2; ++pi) {
        for (int y = 1; y <= 12; ++y) {
            for (int x = 1; x <= 6; ++x) {
                Box box = BoundingBox::boxForAnalysis(pi, x, y);
                drawBoxWithAnalysisResult(surface, box);
            }
        }
    }

    for (int pi = 0; pi < 2; ++pi) {
        Box bs[4] = {
            BoundingBox::boxForAnalysis(pi, NextPuyoPosition::NEXT1_AXIS),
            BoundingBox::boxForAnalysis(pi, NextPuyoPosition::NEXT1_CHILD),
            BoundingBox::boxForAnalysis(pi, NextPuyoPosition::NEXT2_AXIS),
            BoundingBox::boxForAnalysis(pi, NextPuyoPosition::NEXT2_CHILD)
        };

        for (int i = 0; i < 4; ++i) {
            drawBoxWithAnalysisResult(surface, bs[i]);
        }
    }

    drawBoxWithAnalysisResult(surface, BoundingBox::boxForAnalysis(BoundingBox::Region::LEVEL_SELECT_1P));
    drawBoxWithAnalysisResult(surface, BoundingBox::boxForAnalysis(BoundingBox::Region::LEVEL_SELECT_2P));
    drawBoxWithAnalysisResult(surface, BoundingBox::boxForAnalysis(BoundingBox::Region::GAME_FINISHED));
}

void ACAnalyzer::drawBoxWithAnalysisResult(SDL_Surface* surface, const Box& box)
{
    for (int by = box.sy; by < box.dy; ++by) {
        for (int bx = box.sx; bx < box.dx; ++bx) {
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
RealColor ACAnalyzer::estimateRealColor(const HSV& hsv)
{
    return toRealColor(hsv);
}
