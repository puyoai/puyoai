#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "core/server/game_state.h"
#include "duel/cui.h"

using std::string;
using std::cout;

namespace {

const char C_RED[] = "\x1b[41m";
const char C_BLUE[] = "\x1b[44m";
const char C_GREEN[] = "\x1b[42m";
const char C_YELLOW[] = "\x1b[43m";
const char C_BLACK[] = "\x1b[49m";
const char CLEAR_LINE[] = "\x1b[K";

string locate(int x, int y)
{
    std::stringstream ss;
    ss << "\x1b[" << y << ";" << x << "H";
    return ss.str();
}

string locate(int playerId, int x, int y)
{
    int posX = 1 + 30 * playerId;
    int posY = 1;
    return locate(posX + x, posY + y);
}

string puyoText(PuyoColor color, int y = 0)
{
    string text;
    if (color == PuyoColor::OJAMA) {
        text = "@@";
    } else if (color == PuyoColor::WALL) {
        text = "##";
    } else {
        if (y == 13)
            text = "__";
        else
            text = "  ";
    }

    string colorCode;
    switch (color) {
    case PuyoColor::RED:
        colorCode = C_RED;
        break;
    case PuyoColor::BLUE:
        colorCode = C_BLUE;
        break;
    case PuyoColor::GREEN:
        colorCode = C_GREEN;
        break;
    case PuyoColor::YELLOW:
        colorCode = C_YELLOW;
        break;
    default:
        colorCode = C_BLACK;
        break;
    }

    return colorCode + text + C_BLACK;
}

} // namespace

void Cui::clear()
{
    cout << "\x1b[2J" << std::flush;
    printPuyoCache_.clear();
    printTextCache_.clear();
}

void Cui::newGameWillStart()
{
    cout << locate(1, 1 + FieldConstant::MAP_HEIGHT + 3)
         << "\x1B[0K"
         << locate(1, 1 + FieldConstant::MAP_HEIGHT + 4)
         << "\x1B[0K" << std::flush;
    printPuyoCache_.clear();
    printTextCache_.clear();
}

void Cui::onUpdate(const GameState& gameState)
{
    print(0, gameState.playerGameState(0));
    print(1, gameState.playerGameState(1));
    std::cout << std::flush;
}

void Cui::printField(int playerId, const PlayerGameState& pgs)
{
    const Kumipuyo& kumipuyo = pgs.kumipuyoSeq.front();
    const KumipuyoPos& kumipuyoPos = pgs.kumipuyoPos;

    for (int y = 0; y < FieldConstant::MAP_HEIGHT; y++) {
        for (int x = 0; x < FieldConstant::MAP_WIDTH; x++) {
            PuyoColor color = pgs.field.color(x, y);
            if (pgs.playable) {
                if (x == kumipuyoPos.axisX() && y == kumipuyoPos.axisY())
                    color = kumipuyo.axis;
                if (x == kumipuyoPos.childX() && y == kumipuyoPos.childY())
                    color = kumipuyo.child;
            }

            printPuyo(locate(playerId, x * 2, FieldConstant::MAP_HEIGHT - y),
                      puyoText(color, y));
        }
    }
}

void Cui::printNextPuyo(int playerId, const PlayerGameState& pgs)
{
    static const NextPuyoPosition npp[] = {
        NextPuyoPosition::NEXT1_CHILD,
        NextPuyoPosition::NEXT1_AXIS,
        NextPuyoPosition::NEXT2_CHILD,
        NextPuyoPosition::NEXT2_AXIS,
    };

    // Next puyo info
    for (int i = 0; i < 4; ++i) {
        printPuyo(locate(playerId, 9 * 2, 3 + i + (i / 2)),
                  puyoText(pgs.kumipuyoSeq.color(npp[i])));
    }
}

void Cui::printMessage(int playerId, const std::string& message)
{
    if (message.empty())
        return;

    printText(locate(1, 1 + FieldConstant::MAP_HEIGHT + 3 + playerId) + CLEAR_LINE, message);
}

void Cui::print(int playerId, const PlayerGameState& pgs)
{
    printField(playerId, pgs);
    printOjamaPuyo(playerId, pgs);
    printNextPuyo(playerId, pgs);
    printMessage(playerId, pgs.message);
    printScore(playerId, pgs.score);

    // Set cursor
    cout << locate(0, FieldConstant::MAP_HEIGHT + 3);
}

void Cui::printScore(int playerId, int score)
{
    std::ostringstream ss;
    ss << std::setw(10) << score;
    printText(locate(playerId, 0, FieldConstant::MAP_HEIGHT + 1),
              ss.str());
}

void Cui::printOjamaPuyo(int playerId, const PlayerGameState& pgs)
{
    std::ostringstream ss;
    ss << pgs.fixedOjama
       << "(" << pgs.pendingOjama << ")";
    printText(locate(playerId, 0, 0), ss.str());
}

void Cui::printPuyo(const string& location, const string& text)
{
    auto& prev = printPuyoCache_[location];
    if (prev == text)
        return;
    cout << location << text;
    prev = text;
}

void Cui::printText(const string& location, const string& text)
{
    auto& prev = printTextCache_[location];
    if (prev == text)
        return;

    cout << location << text;
    for (size_t i = prev.size(); i < text.size(); ++i)
        cout << ' ';
    prev = text;
}
