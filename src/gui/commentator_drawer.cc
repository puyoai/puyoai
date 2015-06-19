#include "gui/commentator_drawer.h"

#include "base/strings.h"
#include "gui/screen.h"
#include "gui/unique_sdl_surface.h"
#include "gui/util.h"

using namespace std;

namespace {

static void drawNumber(Screen* screen, const Box& b, int n)
{
    SDL_Surface* surface = screen->surface();
    SDL_Color color;
    color.r = color.g = color.b = 255;

    ostringstream oss;
    oss << n;

    UniqueSDLSurface s(makeUniqueSDLSurface(TTF_RenderUTF8_Blended(screen->font(), oss.str().c_str(), color)));

    SDL_Rect dr;
    dr.x = b.dx - s->w;
    dr.y = b.dy - s->h;
    dr.x += 3;
    dr.w = 8;
    dr.h = 20;

    if (screen->surface()->h < dr.y + dr.h)
        return;

    SDL_FillRect(surface, &dr, SDL_MapRGB(s->format, 1, 1, 1));
    SDL_BlitSurface(s.get(), NULL, surface, &dr);
}

static void drawText(Screen* screen, const char* msg, int x, int y)
{
    if (!*msg)
        return;

    SDL_Surface* surface = screen->surface();

    SDL_Color fg;
    fg.r = fg.g = fg.b = 0;
    UniqueSDLSurface s(makeUniqueSDLSurface(TTF_RenderUTF8_Shaded(screen->font(), msg, fg, screen->bgColor())));

    SDL_Rect dr;
    dr.x = x;
    dr.y = y;
    if (surface->w < x + s->w)
        dr.x = surface->w - s->w;

    SDL_BlitSurface(s.get(), NULL, surface, &dr);
}

static void drawText(Screen* screen, const std::string& str, int x, int y)
{
    drawText(screen, str.c_str(), x, y);
}

static void drawTrace(Screen* screen, int pi, const RensaChainTrackResult& result)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 13; ++y) {
            int n = result.erasedAt(x, y);
            if (!n)
                continue;

            Box b = BoundingBox::boxForDraw(pi, x, y);
            b.moveOffset(screen->mainBox().sx, screen->mainBox().sy);
            drawNumber(screen, b, n);
        }
    }
}

}  // anonymous namespace

CommentatorDrawer::CommentatorDrawer()
{
}

CommentatorDrawer::~CommentatorDrawer()
{
}

void CommentatorDrawer::onCommentatorResultUpdate(const CommentatorResult& result)
{
    lock_guard<mutex> lock(mu_);
    result_ = result;
}

void CommentatorDrawer::draw(Screen* screen)
{
    CommentatorResult result;
    {
        lock_guard<mutex> lock(mu_);
        result = result_;
    }

    drawCommentSurface(screen, result, 0);
    drawCommentSurface(screen, result, 1);
    drawMainChain(screen, result);
}

void CommentatorDrawer::drawMainChain(Screen* screen, const CommentatorResult& result) const
{
    for (int pi = 0; pi < 2; pi++) {
        if (result.firingChain[pi].chains() > 0) {
            drawTrace(screen, pi, result.firingChain[pi].trackResult);
            continue;
        }

        if (result.fireableMainChain[pi].chains() > 0) {
            drawTrace(screen, pi, result.fireableMainChain[pi].trackResult);
            continue;
        }
    }
}

void CommentatorDrawer::drawCommentSurface(Screen* screen, const CommentatorResult& result, int pi) const
{
    // TODO(mayah): assert lock is acquired.

    TTF_Font* font = screen->font();
    if (!font)
        return;

    // What is 7?
    int LX = 7 + screen->mainBox().dx * pi;
    int LX2 = 20 + screen->mainBox().dx * pi;
    int LH = 20;

    drawText(screen, "本線", LX, LH * 2);
    if (result.fireableMainChain[pi].chains() > 0) {
        int chains = result.fireableMainChain[pi].chains();
        int score = result.fireableMainChain[pi].score();
        drawText(screen,
                 to_string(chains) + "連鎖" + to_string(score) + "点",
                 LX2, LH * 3);
    }
    drawText(screen, "発火可能潰し", LX, LH * 5);
    if (result.fireableTsubushiChain[pi].chains() > 0) {
        int chains = result.fireableTsubushiChain[pi].chains();
        int score = result.fireableTsubushiChain[pi].score();
        int frames = result.fireableTsubushiChain[pi].totalFrames();
        drawText(screen,
                 to_string(chains) + "連鎖" + to_string(score) + "点",
                 LX2, LH * 6);
        drawText(screen,
                 to_string(score / 70) + "個" + to_string(frames) + "フレーム",
                 LX2, LH * 7);
    }
    drawText(screen, "発火中/最終発火", LX, LH * 9);
    if (result.firingChain[pi].chains() > 0) {
        int chains = result.firingChain[pi].chains();
        int score = result.firingChain[pi].score();
        drawText(screen,
                 to_string(chains) + "連鎖" + to_string(score) + "点",
                 LX2, LH * 10);
    }

    int offsetY = screen->mainBox().dy + 20;
    for (int pi = 0; pi < 2; ++pi) {
        for (const string& s : strings::split(result.message[pi], '\n')) {
            drawText(screen, s.c_str(), 20, offsetY);
            offsetY += LH;
        }
    }

    for (const auto& msg : result.events[pi]) {
        drawText(screen, msg.c_str(), LX, offsetY);
        offsetY += LH;
    }
}
